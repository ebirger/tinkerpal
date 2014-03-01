_stack_size = 4K;

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

#include "platform/arm/cortex-m.ld.S"
