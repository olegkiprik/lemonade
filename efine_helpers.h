/*
	Helpers to make C more dependable
*/

#if !defined(EFINE_HELPERS_H_SENTRY)
#define EFINE_HELPERS_H_SENTRY
#include <assert.h>
#include <stddef.h>

/* static or extern */
#if !defined(EFINE_DEF)
#define EFINE_DEF static
#endif

/* returns 0 or 1 */
#define EFINE_DO_NOT_CARE_0_OR_1() 0

/* sar instruction */
#define EFINE_FLOOR_DIV_BY_POW_2(type, x, n)                                   \
	(((x) > -1) ? ((x) / ((type)1 << (n)))                                 \
		    : (-((-((x) + 1)) / ((type)1 << (n))) - 1))

#if defined(NDEBUG)
#define EFINE_NDEBUG (1 == 1)
#else
#define EFINE_NDEBUG (0 == 1)
#endif

#define EFINE_ASSUME(x)                                                        \
	do {                                                                   \
		if (EFINE_NDEBUG) {                                            \
			if (!(x)) {                                            \
				__builtin_unreachable();                       \
			}                                                      \
		} else {                                                       \
			assert(x);                                             \
		}                                                              \
	} while (0 == 1)

#if defined(EFINE_NOT_SUPPORTED_VARIABLE_ASSUME_ALIGNED)
#define EFINE_ASSUME_ALIGNED(x, a)                                             \
	((a) == 0x4000	 ? __builtin_assume_aligned((x), 0x4000, 0)            \
	 : (a) == 0x2000 ? __builtin_assume_aligned((x), 0x2000, 0)            \
	 : (a) == 0x1000 ? __builtin_assume_aligned((x), 0x1000, 0)            \
	 : (a) == 0x800	 ? __builtin_assume_aligned((x), 0x800, 0)             \
	 : (a) == 0x400	 ? __builtin_assume_aligned((x), 0x400, 0)             \
	 : (a) == 0x200	 ? __builtin_assume_aligned((x), 0x200, 0)             \
	 : (a) == 0x100	 ? __builtin_assume_aligned((x), 0x100, 0)             \
	 : (a) == 0x80	 ? __builtin_assume_aligned((x), 0x80, 0)              \
	 : (a) == 0x40	 ? __builtin_assume_aligned((x), 0x40, 0)              \
	 : (a) == 0x20	 ? __builtin_assume_aligned((x), 0x20, 0)              \
	 : (a) == 0x10	 ? __builtin_assume_aligned((x), 0x10, 0)              \
	 : (a) == 0x8	 ? __builtin_assume_aligned((x), 0x8, 0)               \
	 : (a) == 0x4	 ? __builtin_assume_aligned((x), 0x4, 0)               \
	 : (a) == 0x2	 ? __builtin_assume_aligned((x), 0x2, 0)               \
			 : (x))
#else
#define EFINE_ASSUME_ALIGNED(x, a) __builtin_assume_aligned((x), (a), 0)
#endif

#endif /* EFINE_HELPERS_H_SENTRY */

/* 2026-05-10, some parts were deleted */
