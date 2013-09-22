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
#ifndef __VFS_H__
#define __VFS_H__

#ifdef CONFIG_VFS

#include "util/tstr.h"

typedef int (*readdir_cb_t)(tstr_t *file_name, void *ctx);

typedef struct {
    tstr_t name;
    int (*file_read)(tstr_t *content, tstr_t *file_name);
    int (*readdir)(tstr_t *path, readdir_cb_t cb, void *ctx);
    void (*init)(void);
    void (*uninit)(void);
} fs_t;

static inline int vfs_is_root_path(tstr_t *path)
{
    return path->len == 0 || (path->len == 1 && (path->value[0] == '.' || 
        path->value[0] == '/'));
}

#define VFS_FLAGS_ANY_FS 0x1

int vfs_file_read(tstr_t *content, tstr_t *file_name, int flags);
int vfs_readdir(tstr_t *path, readdir_cb_t cb, void *ctx);

void vfs_init(void);
void vfs_uninit(void);

#else

static inline void vfs_init(void) { }
static inline void vfs_uninit(void) { }

#endif

#endif
