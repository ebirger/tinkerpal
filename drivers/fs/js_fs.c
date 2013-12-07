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
#include "js/js_obj.h"
#include "drivers/fs/vfs.h"

#define Sexception_path_not_found S("Exception: Path not found")

int do_read_file_sync(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    tstr_t content, path;
    int rc;

    tp_assert(argc == 2);

    path = obj_get_str(argv[1]);

    if (vfs_file_read(&content, &path, 0))
    {
	rc = throw_exception(ret, &Sexception_path_not_found);
	goto Exit;
    }

    *ret = string_new(tstr_dup(content));
    rc = 0;

Exit:
    tstr_free(&content);
    tstr_free(&path);
    return rc;
}

int do_write_file_sync(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    tstr_t path, data;
    int rc;

    tp_assert(argc == 3);

    path = obj_get_str(argv[1]);
    data = obj_get_str(argv[2]);

    if (vfs_file_write(&data, &path))
    {
	rc = throw_exception(ret, &Sexception_path_not_found);
	goto Exit;
    }

    *ret = UNDEF;
    rc = 0;

Exit:
    tstr_free(&path);
    tstr_free(&data);
    return rc;
}

static int readdir_cb(tstr_t *file_name, void *ctx)
{
    obj_t *arr = (obj_t *)ctx;

    array_push(arr, string_new(tstr_dup(*file_name)));
    return 0;
}

int do_readdir_sync(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    tstr_t path;
    obj_t *arr;
    int rc;

    tp_assert(argc == 2);

    path = obj_get_str(argv[1]);
    arr = array_new();

    if (vfs_readdir(&path, readdir_cb, arr))
    {
	obj_put(arr);
	rc = throw_exception(ret, &Sexception_path_not_found);
	goto Exit;
    }

    *ret = arr;
    rc = 0;

Exit:
    tstr_free(&path);
    return rc;
}
