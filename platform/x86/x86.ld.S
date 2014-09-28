SECTIONS
{
    /* Load address at 1M */
    . = 1M;

    .text BLOCK(4K) : ALIGN(4K)
    {
        /* Multiboot header */
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    }
    _heap_bottom = .;

    _heap_top = _heap_bottom + 128K;
    
    _stack_top = _heap_top + 4K;

}
