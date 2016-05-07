/* Copyright (c) 2013, Eyal Birger
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "util/event.h"
#include "util/debug.h"
#include "mem/tmalloc.h"
#include "main/console.h"
#include "util/tp_types.h"
#include "platform/platform.h"
#include "boards/board.h"
#include "fs/vfs.h"
#include "js/js.h"
#include "net/net.h"
#include "usb/usbd.h"
#include "apps/app.h"
#include "main/tp.h"
#include "version.h"

static inline void tp_banner(void)
{
    console_printf("TinkerPal version %s\n", TINKERPAL_VERSION);
    if (board.desc)
        console_printf("Running on %s\n", board.desc);
}

static void validate_types(void)
{
    COMPILE_TIME_ASSERT(sizeof(s8) == 1);
    COMPILE_TIME_ASSERT(sizeof(u8) == 1);
    COMPILE_TIME_ASSERT(sizeof(s16) == 2);
    COMPILE_TIME_ASSERT(sizeof(u16) == 2);
    COMPILE_TIME_ASSERT(sizeof(s32) == 4);
    COMPILE_TIME_ASSERT(sizeof(u32) == 4);
    COMPILE_TIME_ASSERT(sizeof(s64) == 8);
    COMPILE_TIME_ASSERT(sizeof(u64) == 8);
}

void tp_init(void)
{
    debugfn_t dbg = {};

    platform_init();
    tmalloc_init();
    console_init();
    validate_types();
    tp_banner();

    dbg.print = console_printf;
    dbg.write = console_write;
    dbg.panic = platform.panic;

    debug_init(&dbg);

    vfs_init();
    js_init();
    net_init();
    usbd_init();

    platform_meminfo();
}

void tp_uninit(void)
{
    usbd_uninit();
    net_uninit();
    js_uninit();
    vfs_uninit();

    tmalloc_uninit();
    platform_uninit();
}

int tp_main(int argc, char *argv[])
{
    tp_init();

    app_start(argc, argv);

    event_loop();

    tp_uninit();
    return 0;
}
