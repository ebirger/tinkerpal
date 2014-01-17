_stack_size = 4K;

/* XXX: platform has in fact 48K RAM */
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 40K
}

#include "platform/arm/cortex-m.ld.S"
