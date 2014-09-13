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
#include "platform/platform.h"
#include "util/tstr.h"
#include "mem/tmalloc.h"
#include "fs/vfs.h"
#include <stdio.h>

static int local_file_read(tstr_t *content, tstr_t *file_name)
{
    FILE *fp;
    size_t nread;
    long fsize;
    char *file_n = NULL;
    int rc = -1;

    file_n = tstr_to_strz(file_name);

    if (!(fp = fopen(file_n, "r")))
    {
        /* Silently fail */
        goto Exit;
    }

    fseek(fp, 0L, SEEK_END);
    if ((fsize = ftell(fp)) < 0)
    {
        tp_err(("Failed ftell() on file %s\n", file_n));
        goto Exit;
    }

    rewind(fp);

    tstr_alloc(content, fsize);
    nread = fread(TPTR(content), 1, fsize, fp);
    if (nread != fsize)
    {
        tp_err(("Read %d/%d from file %S\n", nread, content->len, file_name));
        tstr_free(content);
        goto Exit;
    }

    rc = 0;

Exit:
    if (fp)
        fclose(fp);
    tfree(file_n);
    return rc;
}

static int local_file_write(tstr_t *content, tstr_t *file_name)
{
    FILE *fp;
    size_t nwrote;
    char *file_n = NULL;
    int rc = -1;

    file_n = tstr_to_strz(file_name);

    if (!(fp = fopen(file_n, "w")))
    {
        /* Silently fail */
        goto Exit;
    }

    nwrote = fwrite(TPTR(content), 1, content->len, fp);
    if (nwrote != content->len)
    {
        tp_err(("Wrote %d/%d to file %S\n", nwrote, content->len, file_name));
        goto Exit;
    }

    rc = 0;

Exit:
    if (fp)
        fclose(fp);
    tfree(file_n);
    return rc;
}

int local_readdir(tstr_t *path, readdir_cb_t cb, void *ctx)
{
    tp_err(("Readdir not implemented yet...\n"));
    return -1;
}

static void local_init(void)
{
}

static void local_uninit(void)
{
}

const fs_t local_fs = {
    .name = "Local",
    .init = local_init,
    .uninit = local_uninit,
    .file_read = local_file_read,
    .file_write = local_file_write,
    .readdir = local_readdir,
};
