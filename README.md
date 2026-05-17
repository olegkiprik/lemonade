## Leak Eliminator & Memory Ownership Normalizer & Atrocious Debugging Exposer

Make your code free from Rust!

## Mutation testing

Surround tested code with backtick and dollar sign:

<code>
/* ` */

/* $ */
</code>

Use the Bug Incubator:
<kbd>./bug filename > result</kbd>

## Memory debugging

<code>
/* ... */

#define MEMORY_DEBUGGER_IMPLEMENTATION /* add this line once */
#include "memory_debugger.h"
/* and then */
#include "memory_debugger_macros.h"

int main(void)
{
    void* mem;

    if (0 != frg_debug_mem_init(1, 0x1000)) {
        /* failed to init */
    }

    mem = malloc(9);
    /* check gl_debug_mem_err */

    frg_debug_mem_finish();
    return EXIT_SUCCESS;
}
</code>
