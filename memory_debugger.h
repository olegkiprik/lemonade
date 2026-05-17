/* Compared pointers are converted to size_t! */

/*
	A memory debugger and limiter.

	The idea is borrowed from
	https://gamepipeline.org/forge.html

	Author of Forge: Eskil Steenberg
*/

#if !defined(MEMORY_DEBUGGER_H_SENTRY)
#define MEMORY_DEBUGGER_H_SENTRY

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if !defined(EFINE_NOT_SUPPORTED_STDINT)
#include <stdint.h>
#endif

#if !defined(MEMORY_DEBUGGER_DEF)
#define MEMORY_DEBUGGER_DEF static
#endif

#if !defined(MEMORY_DEBUGGER_GUARD_SIZE)
#define MEMORY_DEBUGGER_GUARD_SIZE 0
#endif

#if !defined(MEMORY_DEBUGGER_GUARD_CHAR)
#define MEMORY_DEBUGGER_GUARD_CHAR 0xCF
#endif

#if !defined(MEMORY_DEBUGGER_FILL_CHAR)
#endif

#if !defined(EFINE_FLAT_MEMORY_MODEL)
#endif

#if !defined(EFINE_NOT_SUPPORTED_ALIGNED_ALLOC)
#endif

typedef struct mem_debug_data_ mem_debug_data;

MEMORY_DEBUGGER_DEF void frg_debug_mem_finish();

/* bytes: total_mem_limit_gross, single_alloc_limit */
MEMORY_DEBUGGER_DEF int frg_debug_mem_init(size_t nr_alloc_limit,
					   size_t total_mem_limit_gross);

MEMORY_DEBUGGER_DEF void *frg_debug_mem_aligned_alloc(size_t alignment,
						      size_t n,
						      const char *file,
						      unsigned long line);

MEMORY_DEBUGGER_DEF void *frg_debug_mem_malloc(size_t n, const char *file,
					       unsigned long line);

MEMORY_DEBUGGER_DEF void frg_debug_mem_free(void *p, const char *file,
					    unsigned long line);

MEMORY_DEBUGGER_DEF void *
frg_debug_mem_calloc(size_t n, size_t m, const char *file, unsigned long line);

MEMORY_DEBUGGER_DEF void *
frg_debug_mem_realloc(void *p, size_t n, const char *file, unsigned long line);

MEMORY_DEBUGGER_DEF void frg_debug_mem_get_info(
    const mem_debug_data *data, size_t *out_total_mem_net,
    size_t *out_total_mem_gross, void **out_mem, size_t *out_mem_uint,
    const char **out_file, unsigned long *out_line, size_t *out_size,
    const char **out_file_free, const char **out_file_double_free,
    unsigned long *out_line_free, unsigned long *out_line_double_free,
    size_t *out_alignment);

MEMORY_DEBUGGER_DEF const mem_debug_data *
frg_debug_mem_get_realloc_data_prev(const mem_debug_data *crt);

typedef enum mem_debug_err_ {
	mem_debug_err_none,
	mem_debug_err_nr_allocs,
	mem_debug_err_total_mem,
	mem_debug_err_zero_alloc,
	mem_debug_err_double_free,
	mem_debug_err_failed_init,
	mem_debug_err_failed_alloc,
	mem_debug_err_invalid_param,
	mem_debug_err_calloc_overflow,
	mem_debug_err_realloc_on_freed,
	mem_debug_err_too_big_to_debug,
	mem_debug_err_free_without_allocs,
	mem_debug_err_free_unknown_pointer,
	mem_debug_err_free_offset_pointer,
	mem_debug_err_realloc_without_allocs,
	mem_debug_err_realloc_unknown_pointer,
	mem_debug_err_realloc_offset_pointer,
	mem_debug_err_not_supported_aligned_alloc
} mem_debug_err;

MEMORY_DEBUGGER_DEF mem_debug_err gl_debug_mem_err = mem_debug_err_none;
MEMORY_DEBUGGER_DEF mem_debug_data *gl_debug_mem_last_data = NULL;

#if defined(MEMORY_DEBUGGER_IMPLEMENTATION)

typedef union mem_debug_data_aligned_or_realloc_union_ {
	size_t alignment;
	mem_debug_data *prev_ptr_data;
} mem_debug_data_aligned_or_realloc_union;

typedef union mem_debug_data_active_or_freed_union_ {
	void *mem;
	size_t mem_uint;
} mem_debug_data_active_or_freed_union;

struct mem_debug_data_ {
	mem_debug_data_active_or_freed_union unn_active_or_freed;
	/* positive -> alignment, negative -> prev_ptr_data */
	size_t size_sm;
	mem_debug_data_aligned_or_realloc_union unn_aligned_or_realloc;
	const void *file_alloc;
	/* null -> mem, other -> mem_uint */
	const void *file_free;
	const void *file_double_free;
	uint_least16_t line_alloc;
	uint_least16_t line_free;
	uint_least16_t line_double_free;
};

static mem_debug_data *gl_priv_debug_mem_data = NULL;
static mem_debug_data *gl_priv_debug_mem_data_next = NULL;
static mem_debug_data *gl_priv_debug_mem_data_first_outside = NULL;
static size_t gl_priv_debug_mem_spent_gross = 0;
static size_t gl_priv_debug_mem_spent_net = 0;
static size_t gl_priv_debug_mem_limit_total_mem = 0;

MEMORY_DEBUGGER_DEF void frg_debug_mem_finish()
{
	free(gl_priv_debug_mem_data);
	gl_priv_debug_mem_data = NULL;
	gl_priv_debug_mem_data_next = NULL;
	gl_priv_debug_mem_limit_total_mem = 0;
	gl_priv_debug_mem_spent_gross = 0;
	gl_priv_debug_mem_spent_net = 0;
	gl_priv_debug_mem_data_first_outside = NULL;
	gl_debug_mem_err = mem_debug_err_none;
	gl_debug_mem_last_data = NULL;
}

MEMORY_DEBUGGER_DEF int frg_debug_mem_init(size_t nr_alloc_limit,
					   size_t total_mem_limit_gross)
{
	assert(nr_alloc_limit > 0);
	assert(total_mem_limit_gross > 0);
	assert(nr_alloc_limit * sizeof *gl_priv_debug_mem_data <=
	       total_mem_limit_gross);
	assert(gl_priv_debug_mem_data == NULL);

#if defined(EFINE_NOT_SUPPORTED_ALIGNED_ALLOC)
	gl_priv_debug_mem_data =
	    malloc(nr_alloc_limit * sizeof *gl_priv_debug_mem_data);
#else
	gl_priv_debug_mem_data = aligned_alloc(
	    sizeof(void *), nr_alloc_limit * sizeof *gl_priv_debug_mem_data);
#endif

	if (gl_priv_debug_mem_data == NULL) {
		gl_debug_mem_err = mem_debug_err_failed_init;
		return 1;
	}

	gl_priv_debug_mem_data_next = gl_priv_debug_mem_data;
	gl_priv_debug_mem_limit_total_mem = total_mem_limit_gross;
	gl_priv_debug_mem_spent_gross =
	    nr_alloc_limit * sizeof *gl_priv_debug_mem_data;
	gl_priv_debug_mem_data_first_outside =
	    gl_priv_debug_mem_data + nr_alloc_limit;
	return 0;
}

MEMORY_DEBUGGER_DEF void *frg_debug_mem_aligned_alloc(size_t alignment,
						      size_t n,
						      const char *file,
						      unsigned long line)
{
	void *mem_gross;
	void *mem_net;
	mem_debug_data *crt;
	size_t mem_uint;
	size_t mem_uint_fo;
	size_t guard_size;
	size_t size;

	assert(file != NULL);

	if (alignment <= 0 || n < 0) {
		gl_debug_mem_err = mem_debug_err_invalid_param;
		return NULL;
	}

	if (n == 0) {
		gl_debug_mem_err = mem_debug_err_zero_alloc;
		return NULL;
	}

	if (n > (size_t)PTRDIFF_MAX) {
		gl_debug_mem_err = mem_debug_err_too_big_to_debug;
		return NULL;
	}

	if (gl_priv_debug_mem_data_next ==
	    gl_priv_debug_mem_data_first_outside) {
		gl_debug_mem_err = mem_debug_err_nr_allocs;
		return NULL;
	}

	if (gl_priv_debug_mem_spent_gross + n >
	    gl_priv_debug_mem_limit_total_mem) {
		gl_debug_mem_err = mem_debug_err_total_mem;
		return NULL;
	}

#if defined(EFINE_NOT_SUPPORTED_ALIGNED_ALLOC)
	if (alignment != 1) {
		gl_debug_mem_err = mem_debug_err_not_supported_aligned_alloc;
		return NULL;
	}

	guard_size = MEMORY_DEBUGGER_GUARD_SIZE;
	mem_gross = malloc(n + guard_size * 2);
#else
	guard_size = (MEMORY_DEBUGGER_GUARD_SIZE + alignment - 1) / alignment *
		     alignment;
	mem_gross = aligned_alloc(alignment, n + guard_size * 2);
#endif

	if (mem_gross == NULL) {
		gl_debug_mem_err = mem_debug_err_failed_alloc;
		return NULL;
	}

	mem_net = mem_gross + guard_size;

#if defined(EFINE_FLAT_MEMORY_MODEL)
	mem_uint = (size_t)mem_gross;
	mem_uint_fo = (size_t)((char *)mem_gross + n + guard_size * 2);

	for (crt = gl_priv_debug_mem_data;
	     crt != gl_priv_debug_mem_data_first_outside; ++crt) {
		if (crt->size_sm <= PTRDIFF_MAX) {
			size = crt->size_sm;
		} else {
			size = crt->size_sm - PTRDIFF_MAX - 1;
		}
		if (crt->file_free != NULL &&
		    crt->unn_active_or_freed.mem_uint < mem_uint_fo &&
		    mem_uint < crt->unn_active_or_freed.mem_uint + size) {
			crt->unn_active_or_freed.mem_uint = (size_t)NULL;
		}
	}
#endif

#if defined(MEMORY_DEBUGGER_FILL_CHAR)
	memset(mem_net, MEMORY_DEBUGGER_FILL_CHAR, n);
#endif

	memset(mem_gross, MEMORY_DEBUGGER_GUARD_CHAR, guard_size);
	memset(mem_gross + guard_size + n, MEMORY_DEBUGGER_GUARD_CHAR,
	       guard_size);

	crt = gl_priv_debug_mem_data_next;
	++gl_priv_debug_mem_data_next;
	memset(crt, '\0', sizeof *crt);

	crt->unn_active_or_freed.mem = mem_gross;
	crt->size_sm = n;
	crt->file_alloc = file;
	crt->line_alloc = (uint_least16_t)line;
	crt->file_free = NULL;
	crt->unn_aligned_or_realloc.alignment = alignment;

	gl_priv_debug_mem_spent_gross += n + 2 * guard_size;
	gl_priv_debug_mem_spent_net += n;
	gl_debug_mem_last_data = crt;

	return mem_net;
}

static mem_debug_data *
frg_priv_debug_mem_find(void *p_net, mem_debug_err *out_err, const char *file,
			unsigned long line, int is_realloc)
{
	mem_debug_data *crt;
	size_t alignment;
	size_t size;
	size_t guard_size;
	size_t p_net_int;

	assert(file != NULL);

	p_net_int = (size_t)p_net;

	for (crt = gl_priv_debug_mem_data;
	     crt != gl_priv_debug_mem_data_first_outside; ++crt) {
		if (crt->size_sm <= PTRDIFF_MAX) {
			alignment = crt->unn_aligned_or_realloc.alignment;
			size = crt->size_sm;
		} else {
			alignment = 1;
			size = crt->size_sm - PTRDIFF_MAX - 1;
		}
		guard_size = (MEMORY_DEBUGGER_GUARD_SIZE + alignment - 1) /
			     alignment * alignment;
		if (crt->file_free == NULL) {
			if (p_net_int ==
			    (size_t)((char *)crt->unn_active_or_freed.mem +
				     guard_size)) {
				return crt;
			}
#if defined(EFINE_FLAT_MEMORY_MODEL)
			else if ((size_t)((char *)crt->unn_active_or_freed.mem +
					  guard_size) < p_net_int &&
				 p_net_int <
				     (size_t)((char *)
						  crt->unn_active_or_freed.mem +
					      guard_size * 2 + size)) {
				*out_err =
				    is_realloc == 1
					? mem_debug_err_realloc_offset_pointer
					: mem_debug_err_free_offset_pointer;
				return NULL;
			}
#endif
		} else {
			if (p_net_int ==
			    crt->unn_active_or_freed.mem_uint + guard_size) {
				*out_err = is_realloc == 1
					       ? mem_debug_err_realloc_on_freed
					       : mem_debug_err_double_free;
				if (crt->file_double_free == NULL) {
					crt->file_double_free = file;
					crt->line_double_free = line;
				}
				return NULL;
			}
#if defined(EFINE_FLAT_MEMORY_MODEL)
			else if (crt->unn_active_or_freed.mem_uint +
					 guard_size <=
				     p_net_int &&
				 p_net_int < crt->unn_active_or_freed.mem_uint +
						 guard_size * 2 + size) {
				*out_err = is_realloc == 1
					       ? mem_debug_err_realloc_on_freed
					       : mem_debug_err_double_free;
				return NULL;
			}
#endif
		}
	}

	return NULL;
}

MEMORY_DEBUGGER_DEF void frg_debug_mem_free(void *p_net, const char *file,
					    unsigned long line)
{
	mem_debug_data *crt;
	size_t guard_size;
	size_t size;
	size_t alignment;
	mem_debug_err err;

	assert(file != NULL);

	if (p_net == NULL) {
		return;
	}

	if (gl_priv_debug_mem_data_next == gl_priv_debug_mem_data) {
		gl_debug_mem_err = mem_debug_err_free_without_allocs;
		return;
	}

	err = mem_debug_err_none;
	crt = frg_priv_debug_mem_find(p_net, &err, file, line, 0);
	if (err != mem_debug_err_none) {
		gl_debug_mem_err = err;
		return;
	}

	if (crt == NULL) {
		gl_debug_mem_err = mem_debug_err_free_unknown_pointer;
		return;
	}

	if (crt->size_sm <= PTRDIFF_MAX) {
		alignment = crt->unn_aligned_or_realloc.alignment;
	} else {
		alignment = 1;
	}

	guard_size = (MEMORY_DEBUGGER_GUARD_SIZE + alignment - 1) / alignment *
		     alignment;

	gl_debug_mem_last_data = crt;

	crt->file_free = file;
	crt->line_free = line;

	free(crt->unn_active_or_freed.mem);
#if !defined(EFINE_FLAT_MEMORY_MODEL)
	crt->unn_active_or_freed.mem_uint = (size_t)NULL;
#else
	crt->unn_active_or_freed.mem_uint =
	    (size_t)crt->unn_active_or_freed.mem;
#endif

	size = crt->size_sm <= PTRDIFF_MAX ? crt->size_sm
					   : crt->size_sm - PTRDIFF_MAX - 1;
	gl_priv_debug_mem_spent_gross -= size + guard_size * 2;
	gl_priv_debug_mem_spent_net -= size;
}

MEMORY_DEBUGGER_DEF void *frg_debug_mem_realloc(void *p_net, size_t n,
						const char *file,
						unsigned long line)
{
	void *mem_gross;
	void *mem_net;
	mem_debug_data *crt_old;
	mem_debug_data *crt_new;
	size_t old_size;
	size_t min_size;
	mem_debug_err err;
	size_t guard_size;

	assert(file != NULL);

	if (n < 0) {
		gl_debug_mem_err = mem_debug_err_invalid_param;
		return NULL;
	}

	if (p_net == NULL && n == 0) {
		gl_debug_mem_err = mem_debug_err_zero_alloc;
		return NULL;
	}

	if (p_net == NULL) {
		/* realloc with null */

		/* HACK */
		mem_net = frg_debug_mem_malloc(n, file, line);
		if (mem_net == NULL) {
			return NULL;
		}

		return mem_net;
	}

	if (n == 0) {
		frg_debug_mem_free(p_net, file, line);
		return NULL;
	}

	if (n > (size_t)PTRDIFF_MAX) {
		gl_debug_mem_err = mem_debug_err_too_big_to_debug;
		return NULL;
	}

	if (gl_priv_debug_mem_data_next ==
	    gl_priv_debug_mem_data_first_outside) {
		gl_debug_mem_err = mem_debug_err_nr_allocs;
		return NULL;
	}

	if (gl_priv_debug_mem_spent_gross + n >
	    gl_priv_debug_mem_limit_total_mem) {
		gl_debug_mem_err = mem_debug_err_total_mem;
		return NULL;
	}

	if (gl_priv_debug_mem_data == gl_priv_debug_mem_data_next) {
		gl_debug_mem_err = mem_debug_err_realloc_without_allocs;
		return NULL;
	}

	err = mem_debug_err_none;
	crt_old = frg_priv_debug_mem_find(p_net, &err, file, line, 1);
	if (err != mem_debug_err_none) {
		gl_debug_mem_err = err;
		return NULL;
	}

	if (crt_old == NULL) {
		gl_debug_mem_err = mem_debug_err_realloc_unknown_pointer;
		return NULL;
	}

	old_size = crt_old->size_sm <= PTRDIFF_MAX
		       ? crt_old->size_sm
		       : crt_old->size_sm - PTRDIFF_MAX - 1;

	if (gl_priv_debug_mem_spent_gross - old_size + n >
	    gl_priv_debug_mem_limit_total_mem) {
		gl_debug_mem_err = mem_debug_err_total_mem;
		return NULL;
	}

	guard_size = MEMORY_DEBUGGER_GUARD_SIZE;
	mem_gross = malloc(n + guard_size * 2);

	if (mem_gross == NULL) {
		gl_debug_mem_err = mem_debug_err_failed_alloc;
		return NULL;
	}

	mem_net = (char *)mem_gross + guard_size;

	min_size = old_size < n ? old_size : n;
	memcpy(mem_net, crt_old->unn_active_or_freed.mem, min_size);
#if defined(MEMORY_DEBUGGER_FILL_CHAR)
	memset(mem_net + min_size, MEMORY_DEBUGGER_FILL_CHAR, n - min_size);
#endif
	memset(mem_gross, MEMORY_DEBUGGER_GUARD_CHAR, guard_size);
	memset(mem_gross + guard_size + n, MEMORY_DEBUGGER_GUARD_CHAR,
	       guard_size);

	free(crt_old->unn_active_or_freed.mem);
#if !defined(EFINE_FLAT_MEMORY_MODEL)
	crt_old->unn_active_or_freed.mem_uint = (size_t)NULL;
#else
	crt_old->unn_active_or_freed.mem_uint =
	    (size_t)crt_old->unn_active_or_freed.mem;
#endif

	crt_new = gl_priv_debug_mem_data_next;
	++gl_priv_debug_mem_data_next;

	memset(crt_new, '\0', sizeof *crt_new);
	crt_new->file_alloc = file;
	crt_new->file_free = NULL;
	crt_new->line_alloc = (uint_least16_t)line;
	crt_new->size_sm = n + PTRDIFF_MAX + 1;
	crt_new->unn_aligned_or_realloc.prev_ptr_data = crt_old;
	crt_new->unn_active_or_freed.mem = mem_gross;

	gl_priv_debug_mem_spent_gross -= old_size;
	gl_priv_debug_mem_spent_gross += n;
	gl_priv_debug_mem_spent_net -= old_size;
	gl_priv_debug_mem_spent_net += n;
	gl_debug_mem_last_data = crt_new;
	return mem_net;
}

MEMORY_DEBUGGER_DEF void frg_debug_mem_get_info(
    const mem_debug_data *data, size_t *out_total_mem_net,
    size_t *out_total_mem_gross, void **out_mem, size_t *out_mem_uint,
    const char **out_file, unsigned long *out_line, size_t *out_size,
    const char **out_file_free, const char **out_file_double_free,
    unsigned long *out_line_free, unsigned long *out_line_double_free,
    size_t *out_alignment)
{
	if (out_total_mem_net != NULL) {
		*out_total_mem_net = gl_priv_debug_mem_spent_net;
	}
	if (out_total_mem_gross != NULL) {
		*out_total_mem_gross = gl_priv_debug_mem_spent_gross;
	}
	if (data == NULL) {
		return;
	}
	if (out_mem != NULL) {
		if (data->file_free == NULL) {
			*out_mem = data->unn_active_or_freed.mem;
		} else {
			/* */
		}
	}
	if (out_mem_uint != NULL) {
		if (data->file_free == NULL) {
			*out_mem_uint = (size_t)data->unn_active_or_freed.mem;
		} else {
			*out_mem_uint = data->unn_active_or_freed.mem_uint;
		}
	}
	if (out_file != NULL) {
		*out_file = data->file_alloc;
	}
	if (out_line != NULL) {
		*out_line = data->line_alloc;
	}
	if (out_size != NULL) {
		*out_size = data->size_sm <= PTRDIFF_MAX
				? data->size_sm
				: data->size_sm - PTRDIFF_MAX - 1;
	}
	if (out_file_free != NULL) {
		*out_file_free = data->file_free;
	}
	if (out_file_double_free != NULL) {
		*out_file_double_free = data->file_double_free;
	}
	if (out_line_free != NULL) {
		*out_line_free = data->line_free;
	}
	if (out_line_double_free != NULL) {
		*out_line_double_free = data->line_double_free;
	}
	if (out_alignment != NULL) {
		if (data->size_sm <= PTRDIFF_MAX) {
			*out_alignment = data->unn_aligned_or_realloc.alignment;
		} else {
			*out_alignment = 1;
		}
	}
}

MEMORY_DEBUGGER_DEF const mem_debug_data *
frg_debug_mem_get_realloc_data_prev(const mem_debug_data *crt)
{
	if (crt->size_sm <= PTRDIFF_MAX) {
		return NULL;
	} else {
		return crt->unn_aligned_or_realloc.prev_ptr_data;
	}
}

MEMORY_DEBUGGER_DEF void *frg_debug_mem_malloc(size_t n, const char *file,
					       unsigned long line)
{
	void *mem;

	assert(file != NULL);

	/* HACK */
	mem = frg_debug_mem_aligned_alloc(1, n, file, line);
	if (mem == NULL) {
		return NULL;
	}

	return mem;
}

MEMORY_DEBUGGER_DEF void *
frg_debug_mem_calloc(size_t n, size_t m, const char *file, unsigned long line)
{
	void *mem;

	assert(file != NULL);

	if (n < 0 || m < 0) {
		gl_debug_mem_err = mem_debug_err_invalid_param;
		return NULL;
	}

	if (m != 0 && SIZE_MAX / m < n) {
		gl_debug_mem_err = mem_debug_err_calloc_overflow;
		return NULL;
	}

	/* HACK */
	mem = frg_debug_mem_malloc(n * m, file, line);
	if (mem == NULL) {
		return NULL;
	}

	memset(mem, '\0', n);
	return mem;
}

#endif /* MEMORY_DEBUGGER_IMPLEMENTATION */

#endif /* MEMORY_DEBUGGER_H_SENTRY */
