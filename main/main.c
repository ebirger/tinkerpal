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
#include "drivers/fs/vfs.h"
#include "js/js.h"
#include "net/net.h"
#include "version.h"

extern void app_start(int argc, char *argv[]);

static inline void tp_banner(void)
{
    console_printf("TinkerPal version %s\n", TINKERPAL_VERSION);
    if (board.desc)
        console_printf("Running on %s\n", board.desc);
}

static void validate_types(void)
{
    tp_assert(sizeof(s8) == 1);
    tp_assert(sizeof(u8) == 1);
    tp_assert(sizeof(s16) == 2);
    tp_assert(sizeof(u16) == 2);
    tp_assert(sizeof(s32) == 4);
    tp_assert(sizeof(u32) == 4);
    tp_assert(sizeof(s64) == 8);
    tp_assert(sizeof(u64) == 8);
}

int tp_main(int argc, char *argv[])
{
    debugfn_t dbg = {};

    platform_init();
    tmalloc_init();
    console_init();
    validate_types();
    tp_banner();

    dbg.print = console_printf;
    dbg.panic = platform.panic;

    debug_init(&dbg);

    vfs_init();
    js_init();
    net_init();

    platform_meminfo();

    app_start(argc, argv);
    
    event_loop();

    net_uninit();
    js_uninit();
    vfs_uninit();

    tmalloc_uninit();
    platform_uninit();
    return 0;
}
