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

static const fs_t *get_fs(tstr_t *file_name)
{
    tstr_t fs_name;
    const fs_t **fs;

    fs_name = tstr_cut(file_name, '/');
    foreach_fs(fs)
    {
        if (!tstr_cmp_str(&fs_name, (*fs)->name))
            return *fs;
    }

    tp_err(("VFS: wrong FS type or no FS path given\n"));
    return NULL;
}

int vfs_file_read(tstr_t *content, tstr_t *file_name, int flags)
{
    const fs_t *fs;
    tstr_t file_name_copy;

    if (flags & VFS_FLAGS_ANY_FS)
        return vfs_file_read_anyfs(content, file_name);

    /* get_fs cuts the fs name from the file_name, so we copy it */
    file_name_copy = *file_name;
    if (!(fs = get_fs(&file_name_copy)))
        return -1;

    return fs->file_read(content, &file_name_copy);
}

int vfs_file_write(tstr_t *content, tstr_t *file_name)
{
    const fs_t *fs;
    tstr_t file_name_copy;

    /* get_fs cuts the fs name from the file_name, so we copy it */
    file_name_copy = *file_name;
    if (!(fs = get_fs(&file_name_copy)))
        return -1;

    return fs->file_write(content, &file_name_copy);
}

static void readdir_root(readdir_cb_t cb, void *ctx)
{
    const fs_t **fs;

    foreach_fs(fs)
    {
        tstr_t t;

        tstr_init(&t, (char *)(*fs)->name, strlen((*fs)->name), 0);
        if (cb(&t, ctx))
            break;
    }
}

int vfs_readdir(tstr_t *path, readdir_cb_t cb, void *ctx)
{
    const fs_t *fs;
    tstr_t path_copy;

    if (vfs_is_root_path(path))
    {
        readdir_root(cb, ctx);
        return 0;
    }

    /* get_fs cuts the fs name from the path, so we copy it */
    path_copy = *path;
    if (!(fs = get_fs(&path_copy)))
        return -1;

    return fs->readdir(&path_copy, cb, ctx);
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
