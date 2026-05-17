#if !defined(MEMORY_DEBUGGER_MACROS_H_SENTRY)
#define MEMORY_DEBUGGER_MACROS_H_SENTRY

#define malloc(n) frg_debug_mem_malloc(n, __FILE__, __LINE__)
#define calloc(n, m) frg_debug_mem_calloc(n, m, __FILE__, __LINE__)
#define realloc(x, n) frg_debug_mem_realloc(x, n, __FILE__, __LINE__)
#define aligned_alloc(n, m)                                                    \
	frg_debug_mem_aligned_alloc(n, m, __FILE__, __LINE__)
#define free(x) frg_debug_mem_free(x, __FILE__, __LINE__)

#define MEMORY_DEBUGGER_MACROS_DUMMY_SILENCE_WARNING

#endif /* MEMORY_DEBUGGER_MACROS_H_SENTRY */
