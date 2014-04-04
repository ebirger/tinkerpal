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
#include "mem/tmalloc.h"
#include "js/js_obj.h"
#include "js/js_module.h"
#include "js/js_eval.h"
#include "js/js_types.h"

typedef struct js_module_t {
    struct js_module_t *next;
    tstr_t name;
    tstr_t code;
    obj_t *exports;
} js_module_t;

static js_module_t *js_modules;

static js_module_t *module_load_vfs(tstr_t *mod_name)
{
    js_module_t *mod = NULL;
    tstr_t code;

    if (vfs_file_read(&code, mod_name, VFS_FLAGS_ANY_FS))
        return NULL;

    mod = tmalloc_type(js_module_t);
    mod->code = code;
    mod->name = tstr_dup(*mod_name);
    mod->exports = NULL;
    mod->next = NULL;
    return mod;
}

static js_module_t *module_lookup(tstr_t *mod_name)
{
    js_module_t **mod;

    for (mod = &js_modules; *mod && tstr_cmp(&(*mod)->name, mod_name); 
        mod = &(*mod)->next);
    if (!*mod)
        *mod = module_load_vfs(mod_name);

    return *mod;
}

int module_require(obj_t **ret, tstr_t *mod_name)
{
    int rc;
    js_module_t *mod;

    mod = module_lookup(mod_name);
    if (!mod)
        return -1;

    if (mod->exports)
    {
        *ret = obj_get(mod->exports);
        return 0;
    }

    rc = js_eval_module(ret, &mod->code);
    if (rc == COMPLETION_THROW)
        return rc;

    mod->exports = obj_get(*ret);
    return rc;
}

void modules_uninit(void)
{
    js_module_t *mod;

    while ((mod = js_modules))
    {
        js_modules = js_modules->next;
        obj_put(mod->exports);
        tstr_free(&mod->code);
        tfree(mod);
    }
}

void modules_init(void)
{
}
