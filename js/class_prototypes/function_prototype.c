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
#include "util/tstr_list.h"
#include "js/js_obj.h"
#include "js/js_utils.h"
#include "js/js_eval.h"
#include "js/jsapi_decl.h"

#define Sbound_func INTERNAL_S("bound_func")
#define Sbound_this INTERNAL_S("bound_this")

extern obj_t *global_env;

static int do_null_function(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    *ret = UNDEF;
    return 0;
}

int do_function_prototype_call(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int rc;
    obj_t *saved_this;

    if (argc <= 1)
        return js_invalid_args(ret);

    saved_this = argv[1];
    argv[1] = this; /* this is the called function */
    rc = function_call(ret, saved_this, argc - 1, argv + 1);
    argv[1] = saved_this; /* restore it so it could be freed */
    return rc;
}

int do_function_prototype_apply(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int rc, i;
    function_args_t args;

    if (argc <= 1)
        return js_invalid_args(ret);

    function_args_init(&args, this);

    if (argc == 3)
    {
        array_iter_t iter;

        array_iter_init(&iter, argv[2], 0);
        while (array_iter_next(&iter))
        {
            if (function_args_add(&args, obj_get(iter.obj)))
            {
                obj_put(iter.obj);
                rc = js_invalid_args(ret);
                goto Exit;
            }
        }
        array_iter_uninit(&iter);
    }

    rc = function_call(ret, argv[1], args.argc, args.argv);

Exit:
    for (i = 1; i < args.argc; i++)
        obj_put(args.argv[i]);
    function_args_uninit(&args);
    return rc;
}

static int function_bind_call(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int rc = 0;
    obj_t *bound_func, *bound_this, *wrapper_env, *saved_func;

    wrapper_env = to_function(argv[0])->scope;

    bound_func = obj_get_own_property(NULL, wrapper_env, &Sbound_func);
    if (!(bound_this = obj_get_own_property(NULL, wrapper_env, &Sbound_this)))
        bound_this = obj_get(this);

    saved_func = argv[0];
    argv[0] = bound_func;
    rc = function_call(ret, bound_this, argc, argv);
    argv[0] = saved_func; /* restore it so it could be freed */

    obj_put(bound_func);
    obj_put(bound_this);
    return rc;
}

int do_function_prototype_bind(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    obj_t *wrapper_env;

    wrapper_env = env_new(NULL);
    obj_set_property(wrapper_env, Sbound_func, this);
    if (argc > 1)
        obj_set_property(wrapper_env, Sbound_this, argv[1]);

    *ret = function_new(NULL, NULL, NULL, wrapper_env, function_bind_call);
    obj_put(wrapper_env);
    return 0;
}


int do_function_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    scan_t *code = NULL;
    tstr_t body;
    tstr_list_t *params = NULL;
    call_t call;
    code_free_cb_t code_free_cb = NULL;

    argc--;
    argv++;

    if (!argc)
    {
        call = do_null_function;
        goto Exit;
    }

    if (argc > 1)
    {
        scan_t *scanned_params;
        tstr_t params_raw;

        params_raw = obj_get_str(argv[0]);
        argc--;
        argv++;
        while (argc > 1)
        {
            tstr_t next, sep = S(", "), tmp = {};

            next = obj_get_str(argv[0]);

            tstr_cat(&tmp, &params_raw, &sep);
            tstr_free(&params_raw);
            tstr_cat(&params_raw, &tmp, &next);
            tstr_free(&tmp);
            tstr_free(&next);

            argc--;
            argv++;
        }

        scanned_params = js_scan_init(&params_raw);

        tstr_list_add(&params, &INTERNAL_S("__constructed_func__"));
        if (parse_function_param_list(&params, scanned_params))
        {
            tstr_list_free(&params);
            tstr_free(&params_raw);
            js_scan_uninit(scanned_params);
            return throw_exception(ret, &S("Exception: Parse error"));
        }

        js_scan_uninit(scanned_params);
        tstr_free(&params_raw);
    }

    body = obj_get_str(argv[0]);
    code = _js_scan_init(&body, 1);
    call = call_evaluated_function;
    code_free_cb = evaluated_function_code_free;

Exit:
    *ret = function_new(params, code, code_free_cb, global_env, call);
    return 0;
}
