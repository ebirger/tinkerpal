_stack_size = 4K;

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 1024K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
}

#include "platform/arm/cortex-m.ld.S"
