SECTIONS
{
#ifdef INTERRUPT_SECTION
    .interrupts :
    {
        __vector_table = .;
        . = ALIGN(4);
	KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > INTERRUPTS
#endif
    .text :
    {
        _text = .;
#ifndef INTERRUPT_SECTION
        KEEP(*(.isr_vector))
#endif
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > FLASH

    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM

    .bss : AT(ADDR(.data) + SIZEOF(.data))
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SRAM

    _heap_bottom = .;
    _heap_top = ORIGIN(SRAM) + LENGTH(SRAM) - _stack_size;

    _stack_bottom = _heap_top;
    _stack_top = ORIGIN(SRAM) + LENGTH(SRAM);
}
