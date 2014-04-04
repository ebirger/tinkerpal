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
#include "util/debug.h"
#include "util/tstr.h"
#include "platform/platform_consts.h"
#include "boards/board.h"
#include "drivers/fs/vfs.h"
#include "drivers/mmc/mmc.h"
#include "apps/cli.h"

static int fat_mmc_test_readdir_cb(tstr_t *file_name, void *ctx)
{
    tp_out(("%S\n", file_name));
    return 0;
}

static void fat_mmc_test_process_line(tstr_t *line)
{
    if (!tstr_cmp(line, &S("dir")))
        vfs_readdir(&S("FAT/"), fat_mmc_test_readdir_cb, NULL);

    console_printf("Ok\n");
}

static cli_client_t fat_mmc_test_cli_client = {
    .process_line = fat_mmc_test_process_line,
};

void app_start(int argc, char *argv[])
{
    tp_out(("TinkerPal Application - FAT MMC Test\n"));

    mmc_init(&board.mmc_params);

    cli_start(&fat_mmc_test_cli_client);
}
