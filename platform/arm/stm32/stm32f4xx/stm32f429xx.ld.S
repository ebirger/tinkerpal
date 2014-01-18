_stack_size = 4K;

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 192K
    CCM (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}

#include "platform/arm/cortex-m.ld.S"
