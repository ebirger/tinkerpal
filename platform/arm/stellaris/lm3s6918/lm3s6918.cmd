--retain=g_pfnVectors

#include "autoconf.h"

--heap_size=20480
--stack_size=4096

#define APP_BASE 0x00000000
#define RAM_BASE 0x20000000

/* System memory map */

MEMORY
{
    /* Application stored in and executes from internal flash */
    FLASH (RX) : origin = APP_BASE, length = 0x00040000
    /* Application uses internal RAM for data */
    SRAM (RWX) : origin = 0x20000000, length = 0x00008000
}

/* Section allocation in memory */

SECTIONS
{
    .intvecs:   > APP_BASE
    .text   :   > FLASH
    .const  :   > FLASH
    .cinit  :   > FLASH
    .pinit  :   > FLASH
    .init_array : > FLASH

    .vtable :   > RAM_BASE
    .data   :   > SRAM
    .bss    :   > SRAM
    .sysmem :   > SRAM
    .stack  :   > SRAM
}

_stack_top = __stack + 512;
