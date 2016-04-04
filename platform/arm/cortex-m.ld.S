MEMORY
{
#define ARM_MEMORY_AREA(name, perm, origin, length) \
    name (perm) : ORIGIN = origin, LENGTH = length
#include "platform/chipset.h"
}

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
    } > FLASH

    .ARM.exidx :
    {
	__exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
	__exidx_end = .;
    } > FLASH
	
    _etext = .;

    .data : AT(_etext)
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

#ifdef STACK_RAM_SECTION
    _heap_top = ORIGIN(SRAM) + LENGTH(SRAM);
#else
#define STACK_RAM_SECTION SRAM
    _heap_top = ORIGIN(SRAM) + LENGTH(SRAM) - _stack_size;
#endif

    _stack_top = ORIGIN(STACK_RAM_SECTION) + LENGTH(STACK_RAM_SECTION);
}
