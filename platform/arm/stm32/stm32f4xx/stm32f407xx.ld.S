_stack_size = 4K;

MEMORY
{
    FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    CCM (rwx) : ORIGIN = 0x10000000, LENGTH = 64K
}

#define STACK_RAM_SECTION CCM

#include "platform/arm/cortex-m.ld.S"
