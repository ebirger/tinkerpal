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
#include "fs/vfs.h"

#ifdef CONFIG_FAT_FS
extern const fs_t fat_fs;
#endif
#ifdef CONFIG_LOCAL_FS
extern const fs_t local_fs;
#endif
#ifdef CONFIG_BUILTIN_FS
extern const fs_t builtin_fs;
#endif

static const fs_t *fs_list[] = {
#ifdef CONFIG_FAT_FS
    &fat_fs,
#endif
#ifdef CONFIG_LOCAL_FS
    &local_fs,
#endif
#ifdef CONFIG_BUILTIN_FS
    &builtin_fs,
#endif
    NULL
};

#define foreach_fs(fs) for (fs = fs_list; *fs; fs++)

int vfs_is_root_path(tstr_t *path)
{
    char c;

    if (path->len == 0)
        return 1;

    if (path->len > 1)
        return 0;

    c = tstr_peek(path, 0);
    return c == '.' || c == '/';
}

int vfs_file_read_anyfs(tstr_t *content, tstr_t *file_name)
{
    const fs_t **fs;
    foreach_fs(fs)
    {
        if ((*fs)->file_read && !(*fs)->file_read(content, file_name))
            return 0;
    }
    return -1;
}

static void path_parse(const tstr_t *full_path, tstr_t *fs_name,
    tstr_t *file_path)
{
    int slash_idx;

    slash_idx = tstr_find(full_path, &S("/"));
    if (slash_idx < 0)
    {
        *fs_name = *full_path;
        *file_path = S("");
    }
    else
        tstr_split(full_path, fs_name, file_path, slash_idx, strlen("/"));
}

static const fs_t *get_fs(const tstr_t *fs_name)
{
    const fs_t **fs;

    foreach_fs(fs)
    {
        if (!tstr_cmp_str(fs_name, (*fs)->name))
            return *fs;
    }

    tp_err(("VFS: wrong FS type or no FS path given\n"));
    return NULL;
}

int vfs_file_read(tstr_t *content, tstr_t *file_name, int flags)
{
    const fs_t *fs;
    tstr_t fs_name, file_path;

    if (flags & VFS_FLAGS_ANY_FS)
        return vfs_file_read_anyfs(content, file_name);

    path_parse(file_name, &fs_name, &file_path);
    if (!(fs = get_fs(&fs_name)))
        return -1;

    return fs->file_read(content, &file_path);
}

int vfs_file_write(tstr_t *content, tstr_t *file_name)
{
    const fs_t *fs;
    tstr_t fs_name, file_path;

    path_parse(file_name, &fs_name, &file_path);
    if (!(fs = get_fs(&fs_name)))
        return -1;

    return fs->file_write(content, &file_path);
}

static void readdir_root(readdir_cb_t cb, void *ctx)
{
    const fs_t **fs;

    foreach_fs(fs)
    {
        tstr_t t;

        tstr_init_copy_string(&t, (*fs)->name);
        cb(&t, ctx);
        tstr_free(&t);
    }
}

int vfs_readdir(tstr_t *path, readdir_cb_t cb, void *ctx)
{
    const fs_t *fs;
    tstr_t fs_name, file_path;

    if (vfs_is_root_path(path))
    {
        readdir_root(cb, ctx);
        return 0;
    }

    path_parse(path, &fs_name, &file_path);
    if (!(fs = get_fs(&fs_name)))
        return -1;

    return fs->readdir(&file_path, cb, ctx);
}

void vfs_init(void)
{
    const fs_t **fs;

    tp_out(("VFS Init\n"));
    foreach_fs(fs)
    {
        (*fs)->init();
        tp_out(("Initialized %s FS\n", (*fs)->name));
    }
}

void vfs_uninit(void)
{
    const fs_t **fs;
    tp_out(("VFS Uninit\n"));
    foreach_fs(fs)
    {
        (*fs)->uninit();
        tp_out(("Uninitialized %s FS\n", (*fs)->name));
    }
}
