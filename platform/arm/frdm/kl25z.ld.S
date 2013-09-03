_stack_size = 4K;

MEMORY
{
  INTERRUPTS (rx) : ORIGIN = 0x00000000, LENGTH = 0xC0
  FLASHCFG (rx) : ORIGIN = 0x00000400, LENGTH = 0x10
  FLASH (rx) : ORIGIN = 0x00000800, LENGTH = 128K - 0x800
  SRAM (rwx) : ORIGIN = 0x1FFFF000, LENGTH = 16K /* SRAM */
}
		
#define INTERRUPT_SECTION

#include "platform/arm/cortex-m.ld.S"

SECTIONS
{
    .cfmprotect : AT(ADDR(.interrupts) + SIZEOF(.interrupts))
    {
        . = ALIGN(4);
  	KEEP(*(.cfmconfig)) /* Flash Configuration Field (FCF) */
        . = ALIGN(4);
    } > FLASHCFG
}
