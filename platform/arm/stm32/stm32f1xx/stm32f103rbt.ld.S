_stack_size = 4K;

/* XXX: platform has in fact 48K RAM */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

#include "platform/arm/cortex-m.ld.S"
