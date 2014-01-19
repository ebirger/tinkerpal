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
#include "drivers/fs/vfs.h"
#include "drivers/fs/builtin_fs/builtin_fs.h"

extern const builtin_fs_file_t builtin_fs_files[];

static int builtin_fs_file_read(tstr_t *content, tstr_t *file_name)
{
    const builtin_fs_file_t *f;

    for (f = builtin_fs_files; f->name && tstr_cmp_str(file_name, f->name);
	f++);
    if (!f->name)
    {
	tp_err(("Builtin FS: File %S not found\n", file_name));
	return -1;
    }

    /* No need to dup the tstr as it is builtin */
    tstr_init(content, *f->content, strlen(*f->content), 0);
    return 0;
}

static int builtin_fs_file_write(tstr_t *content, tstr_t *file_name)
{
    return -1; /* Builtin FS is read only */
}
    
int builtin_fs_readdir(tstr_t *path, readdir_cb_t cb, void *ctx)
{
    const builtin_fs_file_t *f;

    if (!vfs_is_root_path(path))
	return -1;

    for (f = builtin_fs_files; f->name; f++)
    {
	tstr_t t;

	tstr_init(&t, (char *)f->name, strlen(f->name), 0);
	cb(&t, ctx);
    }

    return 0;
}

static void builtin_fs_init(void)
{
}

static void builtin_fs_uninit(void)
{
}

const fs_t builtin_fs = {
    .name = "Builtin",
    .init = builtin_fs_init,
    .uninit = builtin_fs_uninit,
    .file_read = builtin_fs_file_read,
    .file_write = builtin_fs_file_write,
    .readdir = builtin_fs_readdir,
};
