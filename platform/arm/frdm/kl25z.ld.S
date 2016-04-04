_stack_size = 4K;

#define PLATFORM_CHIPSET_H "platform/arm/frdm/kl25z.chip"
		
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
