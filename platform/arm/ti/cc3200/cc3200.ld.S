_stack_size = 4K;

MEMORY
{
    SRAM (rwx) : ORIGIN = 0x20004000, LENGTH = 192K
}

/* No Flash for now */
#define FLASH SRAM

#include "platform/arm/cortex-m.ld.S"
