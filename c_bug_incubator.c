/* Not tested on limited RAM! */

#include "efine_helpers.h"
#if !defined(C_BUG_INCUBATOR_H_SENTRY)
#define C_BUG_INCUBATOR_H_SENTRY
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(CACHE_LINE)
#define CACHE_LINE 64
#endif

typedef enum type_mutation_ {
	VI_STATEMENT_REMOVE,
	VI_PLUS_MINUS,
	VI_REMOVE_MINUS,
	VI_EQUAL_NEGATE,
	VI_COMPARE_MIRROR,
	VI_AND_OR,
	VI_SHIFT_MIRROR,
	VI_INC_DEC,
	VI_REMOVE_EXCLAMATION,
	VI_REMOVE_PLUS_MINUS_ONE,
	VI_ADD_BREAK_CONTINUE,
	VI_REMOVE_ELSE,
	VI_N
} type_mutation;

/* the initial position is at the beginning, the final one is at the end */
int analyze_file_limits(FILE *fd, long *out_max_ncolumns, long *out_nr_lines)
{
	long crt_ncolumns;
	long crt_max_ncolumns;
	long crt_nr_lines;
	char c;
	int c_int;

	assert(out_max_ncolumns != NULL);
	assert(out_nr_lines != NULL);

	crt_nr_lines = 0;
	crt_ncolumns = 0;
	crt_max_ncolumns = 0;
	while (1 == 1) {
		if (EOF == (c_int = fgetc_unlocked(fd))) {
			goto l_finish;
		}

		c = (char)c_int;
		if (c == '\n') {
			if (crt_max_ncolumns < crt_ncolumns) {
				crt_max_ncolumns = crt_ncolumns;
			}
			crt_ncolumns = 0;
			++crt_nr_lines;
		} else {
			++crt_ncolumns;
		}
	}

l_finish:
	if (0 != ferror(fd)) {
		perror("\nFailed to analyze the file \n");
		return 1;
	}

	if (crt_max_ncolumns < crt_ncolumns) {
		crt_max_ncolumns = crt_ncolumns;
	}

	*out_max_ncolumns = crt_max_ncolumns;
	*out_nr_lines = crt_nr_lines;
	return 0;
}

/* non-used memory is set to null terminator */
int read_file(FILE *fd, char *p_first, long nr_lines, long max_ncolumns)
{
	int c_int;
	char c;
	int file_corrupted;
	long crt_column;
	long crt_line;
	char *p_out;

	assert(p_first != NULL);

	file_corrupted = 0;
	crt_column = 0;
	crt_line = 0;
	p_out = p_first;

	while (1 == 1) {
		if (EOF == (c_int = fgetc_unlocked(fd))) {
			perror("\nFailed fgetc while scanning the file\n");
			file_corrupted = 1;
			goto l_finish;
		}

		c = (char)c_int;

		if (c == '\n') {
			while (crt_column != max_ncolumns) {
				*p_out = '\0';
				++p_out;
				++crt_column;
			}
			++crt_line;
			if (crt_line == nr_lines) {
				break;
			}
			crt_column = 0;
			p_out = p_first + crt_line * max_ncolumns + crt_column;
		} else {
			if (crt_column == max_ncolumns) {
				file_corrupted = 1;
				goto l_finish;
			}
			*p_out = c;
			++p_out;
			++crt_column;
		}
	}

l_finish:

	if (file_corrupted == 1) {
		(void)fputs("\nThe file was corrupted after the first scan\n",
			    stderr);
		return 1;
	}

	return 0;
}

int vi_remove_else(char *src, long max_ncolumns, long nr_lines)
{
	int ignore;
	int state;
	char *src_fo;
	char *p;
	long r;
	long n;
	char *target;
	int i;
	char *keyword;

	ignore = 1;
	src_fo = src + max_ncolumns * nr_lines;
	state = 0;
	n = 0;
	keyword = "else";

	/* analysis */

	for (p = src; p != src_fo; ++p) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			continue;
		}

		if (0 == 1) {

		} else if (state == 0 && !isalpha(*p) && *p != '_') {
			state = 1;
		} else if (state == 1 && *p == keyword[state - 1]) {
			state = 2;
		} else if (state == 2 && *p == keyword[state - 1]) {
			state = 3;
		} else if (state == 3 && *p == keyword[state - 1]) {
			state = 4;
		} else if (state == 4 && *p == keyword[state - 1]) {
			state = 5;
		} else if (state == 5 && !isalpha(*p) && *p != '_') {
			state = 1;
			++n;
		} else {
			state = 0;
		}
	}

	if (n == 0) {
		return 1;
	}

	r = rand() % n;

	/* target */

	p = src;
	ignore = 1;
	state = 0;
	while (1 == 1) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			goto l_ignore;
		}

		if (0 == 1) {

		} else if (state == 0 && !isalpha(*p) && *p != '_') {
			state = 1;
		} else if (state == 1 && *p == keyword[state - 1]) {
			state = 2;
			target = p;
		} else if (state == 2 && *p == keyword[state - 1]) {
			state = 3;
		} else if (state == 3 && *p == keyword[state - 1]) {
			state = 4;
		} else if (state == 4 && *p == keyword[state - 1]) {
			state = 5;
		} else if (state == 5 && !isalpha(*p) && *p != '_') {
			state = 1;
			if (r == 0) {
				break;
			}
			--r;
		} else {
			state = 0;
		}
	l_ignore:
		++p;
	}

	for (i = 0; i < strlen(keyword); ++i) {
		*target = ' ';
		++target;
	}

	return 0;
}

int vi_add_break_continue(char *src, long max_ncolumns, long nr_lines)
{
	int state;
	char *src_fo;
	char *p;
	long r;
	long n;
	int ignore;
	char *target;
	char *statement;

	int debug_value;

	debug_value = 0;

	src_fo = src + max_ncolumns * nr_lines;
	state = 0;
	n = 0;

	ignore = 1;
	r = rand() % 2;
	statement = r == 0 ? "break;" : "continue;";

	/* analysis */

	for (p = src; p != src_fo; ++p) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			continue;
		}

		if (0 == 1) {

		} else if (state >= 0 && state + 1 < strlen(statement) &&
			   *p == '\0') {
			++state;
		} else if (state + 1 == strlen(statement) && *p == '\0') {
			++state;
			++n;
		} else if (*p == '\0') {

		} else {
			state = 0;
		}
	}

	if (n == 0) {
		return 1;
	}

	r = rand() % n;

	/* target */

	ignore = 1;
	p = src;
	state = 0;
	while (1 == 1) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			goto l_ignore;
		}

		if (0 == 1) {

		} else if (state >= 0 && state + 1 < strlen(statement) &&
			   *p == '\0') {
			++state;
			target = p;
			debug_value = 1;
		} else if (state + 1 == strlen(statement) && *p == '\0') {
			++state;
			if (r == 0) {
				EFINE_ASSUME(debug_value == 1);
				memcpy(target, statement, strlen(statement));
				break;
			}
			--r;
		} else if (*p == '\0') {

		} else {
			state = 0;
		}
	l_ignore:
		++p;
	}

	return 0;
}

int vi_remove_plus_minus_one(char *src, long max_ncolumns, long nr_lines)
{
	char *p;
	int state;
	long n;
	long r;
	char *target;
	int ignore;

	n = 0;
	state = 0;
	ignore = 1;
	for (p = src; p != src + max_ncolumns * nr_lines; ++p) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			continue;
		}

		if (p == src + max_ncolumns * nr_lines) {
			break;
		}

		if (0 == 1) {
		} else if (state == 0 && (*p == '+' || *p == '-')) {
			state = 1;
		} else if (state == 1) {
			if (*p == '\0') {

			} else if (*p == '1') {
				++n;
				state = 0;
			} else {
				state = 0;
			}
		}
	}

	if (n == 0) {
		return 1;
	}

	r = rand() % n;

	ignore = 1;
	p = src;
	assert(state == 0);
	state = 0;
	while (1 == 1) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			goto l_ignore_2;
		}

		if (0 == 1) {

		} else if (state == 0 && (*p == '+' || *p == '-')) {
			target = p;
			state = 1;
		} else if (state == 1) {
			if (*p == '\0') {

			} else if (*p == '1') {
				if (r == 0) {
					*target = '\0';
					*p = '\0';
					return 0;
				}
				--r;
				state = 0;
			} else {
				state = 0;
			}
		}
	l_ignore_2:
		++p;
	}

	assert(0 == 1);
	return 1;
}

int vi_remove_char(char *src, long max_ncolumns, long nr_lines, char c)
{
	long n;
	long r;
	char *p;
	int ignore;

	ignore = 1;
	n = 0;
	for (p = src; p != src + max_ncolumns * nr_lines; ++p) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			continue;
		}

		if (p == src + max_ncolumns * nr_lines) {
			break;
		}
		if (*p == c) {
			++n;
		}
	}

	if (n == 0) {
		return 1;
	}

	r = rand() % n;

	ignore = 1;
	p = src;
	while (1 == 1) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			goto l_ignore_2;
		}

		if (*p == c) {
			if (r == 0) {
				*p = '\0';
				return 0;
			}
			--r;
		}
	l_ignore_2:
		++p;
	}

	return 0;
}

int vi_exchange_repeated_char(char *src, long max_ncolumns, long nr_lines,
			      char a, char b, int nrepeat)
{
	long r;
	long n;
	char *p;
	int state;
	int opposite;
	int ignore;

	ignore = 1;
	opposite = rand() % 2;
	n = 0;
	state = 0;

	for (p = src; p != src + max_ncolumns * nr_lines; ++p) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			continue;
		}

		if (opposite == 1 && *p == a || opposite == 0 && *p == b) {
			if (state + 1 < nrepeat) {
				++state;
			} else {
				++n;
			}
		} else {
			state = 0;
		}
	}

	if (n == 0) {
		return 1;
	}

	r = rand() % n;
	p = src;
	ignore = 1;
	state = 0;
	while (1 == 1) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			goto l_ignore;
		}

		if (opposite == 1 && *p == a || opposite == 0 && *p == b) {
			if (state + 1 < nrepeat) {
				++state;
			} else if (r == 0) {
				while (nrepeat != 0) {
					*p = opposite == 1 ? b : a;
					--p;
					--nrepeat;
				}
				return 0;
			} else {
				--r;
			}
		} else {
			state = 0;
		}
	l_ignore:
		++p;
	}

	return 0;
}

int vi_statement_remove(char *src, long max_ncolumns, long nr_lines)
{
	long n;
	long r;
	char *p;
	char *target;
	int ignore;

	ignore = 1;
	n = 0;
	for (p = src; p != src + max_ncolumns * nr_lines; ++p) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			continue;
		}

		if (*p == ';') {
			++n;
		}
	}

	if (n == 0) {
		return 1;
	}

	r = rand() % n;
	p = src;
	ignore = 1;
	target = p;
	while (1 == 1) {
		if (*p == '`' && ignore == 1) {
			ignore = 0;
		} else if (*p == '$' && ignore == 0) {
			ignore = 1;
		}

		if (ignore == 1) {
			goto l_ignore;
		}

		if (*p == ';') {
			if (r == 0) {
				while (target != p && *target != '$') {
					*target = '\0';
					++target;
				}
				break;
			}
			--r;
			target = p + 1;
		}
	l_ignore:
		++p;
	}

	return 0;
}

/* always returns 0 */
int bug_incubator_work(char *src, long max_ncolumns, long nr_lines)
{
	int type;

	assert(VI_N > 0);
	type = rand() % VI_N;

	switch (type) {
	case VI_STATEMENT_REMOVE:
		return vi_statement_remove(src, max_ncolumns, nr_lines);
	case VI_PLUS_MINUS:
		return vi_exchange_repeated_char(src, max_ncolumns, nr_lines,
						 '+', '-', 1);
	case VI_REMOVE_MINUS:
		return vi_remove_char(src, max_ncolumns, nr_lines, '-');
	case VI_EQUAL_NEGATE:
		return vi_exchange_repeated_char(src, max_ncolumns, nr_lines,
						 '=', '!', 1);
	case VI_COMPARE_MIRROR:
		return vi_exchange_repeated_char(src, max_ncolumns, nr_lines,
						 '<', '>', 1);
	case VI_AND_OR:
		return vi_exchange_repeated_char(src, max_ncolumns, nr_lines,
						 '&', '|', 2);
	case VI_SHIFT_MIRROR:
		return vi_exchange_repeated_char(src, max_ncolumns, nr_lines,
						 '<', '>', 2);
	case VI_INC_DEC:
		return vi_exchange_repeated_char(src, max_ncolumns, nr_lines,
						 '+', '-', 2);
	case VI_REMOVE_EXCLAMATION:
		return vi_remove_char(src, max_ncolumns, nr_lines, '!');
	case VI_REMOVE_PLUS_MINUS_ONE:
		return vi_remove_plus_minus_one(src, max_ncolumns, nr_lines);
	case VI_ADD_BREAK_CONTINUE:
		return vi_add_break_continue(src, max_ncolumns, nr_lines);
	case VI_REMOVE_ELSE:
		return vi_remove_else(src, max_ncolumns, nr_lines);
	default:
		assert(0 == 1);
		break;
	}

	return 0;
}

/* straight to fo */
void char_replace(char *p, char *fo, char from, char to, ptrdiff_t stride)
{
	assert(stride < 0 && p >= fo || stride > 0 && p <= fo);

	while (p != fo) {
		if (*p == from) {
			*p = to;
		}
		p += stride;
	}
}

int main(int argc, char **argv)
{
	FILE *fd;
	const char *filename;
	long max_ncolumns;
	long nr_lines;
	void *src_first_handle; /* alignment is max_ncolumns */
	char *src_first;
	char *src_first_outside;
	int result;
	int no_mutation;

	if (strlen("") != 0) {
		(void)fputs("\nUnsupported standard library implementation\n",
			    stderr);
		return EXIT_FAILURE;
	}

	src_first_handle = NULL;
	result = 0;

	if (argc < 2) {
		(void)fputs("\nPlease select a file\n", stderr);
		return EXIT_FAILURE;
	}

	no_mutation = argc > 2 ? 1 : 0;

	filename = argv[1];

	fd = fopen(filename, "rb");
	if (fd == NULL) {
		perror("\nFailed to open the file\n");
		return EXIT_FAILURE;
	}

	flockfile(fd);

	if (0 != analyze_file_limits(fd, &max_ncolumns, &nr_lines)) {
		(void)fputs("\nFailed to analyze the file\n", stderr);
		result = 1;
		goto l_close_file;
	}

	if (max_ncolumns == 0 || nr_lines == 0) {
		(void)fputs("\nThe file is empty\n", stderr);
		result = 1;
		goto l_close_file;
	}

	if (0 != fseek(fd, 0, SEEK_SET)) {
		perror("\nFailed to set position in the file\n");
		result = 1;
		goto l_close_file;
	}

	/* enhance alignment */
	max_ncolumns = CACHE_LINE + (max_ncolumns + CACHE_LINE - 1) /
					CACHE_LINE * CACHE_LINE;

	src_first_handle = aligned_alloc(
	    CACHE_LINE, sizeof *src_first * max_ncolumns * nr_lines);
	if (src_first_handle == NULL) {
		/*
			Quit immediately.
			Nothing shall be written to stderr when
			there are memory issues.
			To be aware of them, always check the returned value.
		*/
		funlockfile(fd);
		(void)fclose(fd);
		return EXIT_FAILURE;
	}

	src_first = (char *)src_first_handle;
	src_first_outside = src_first + max_ncolumns * nr_lines;

	if (0 != read_file(fd, src_first, nr_lines, max_ncolumns)) {
		(void)fputs("\nFailed to scan the file\n", stderr);
		result = 1;
		goto l_close_file;
	}

l_close_file:
	funlockfile(fd);
	if (0 != fclose(fd)) {
		perror("\nFailed to close the file\n");
		result = 1;
	}
	if (result != 0) {
		goto l_finish;
	}

	char_replace(src_first, src_first_outside, ' ', '\0', 1);
	char_replace(src_first + max_ncolumns - 1,
		     src_first_outside + max_ncolumns - 1, '\0', '\n',
		     max_ncolumns);

	if (no_mutation == 0) {
		srand((unsigned)((unsigned long)clock() %
				 ((unsigned long)UINT_MAX + 1)));
		if (0 !=
		    bug_incubator_work(src_first, max_ncolumns, nr_lines)) {
			(void)fputs("\nFailed to add mutation\n", stderr);
			result = 1;
			goto l_finish;
		}
	}

	char_replace(src_first, src_first_outside, '\0', ' ', 1);

	flockfile(stdout);
	if (src_first_outside - src_first !=
	    fwrite_unlocked(src_first, 1, src_first_outside - src_first,
			    stdout)) {
		(void)fputs("\nFailed to write the result\n", stderr);
		result = 1;
		goto l_stdout_finish;
	}

l_stdout_finish:
	funlockfile(stdout);
l_finish:
	free(src_first_handle);
	return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif /* C_BUG_INCUBATOR_H_SENTRY */
