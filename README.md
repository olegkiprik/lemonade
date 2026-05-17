## Leak Eliminator & Memory Ownership Normalizer & Atrocious Debugging Exposer

Make your code free from Rust!

## Mutation testing

Surround tested code with the following comments:

<code>
/* ` add mutation */

/* ... */

/* $ no mutation */
</code>

Use the Bug Incubator:
<kbd>./bug filename > result</kbd>

## Memory debugging

<code>
void* mem;

if (0 != frg_debug_mem_init(1, 0x1000)) {
    /* failed to init */
}

mem = malloc(9);
/* check gl_debug_mem_err */

frg_debug_mem_finish();
</code>
