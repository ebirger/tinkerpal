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
#include "js/js_eval.h"

extern obj_t *global_env;

static int do_null_function(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    *ret = UNDEF;
    return 0;
}

int do_function_prototype_call(obj_t **ret, function_t *func, 
    obj_t *this, int argc, obj_t *argv[])
{
    tp_assert(argc > 0);

    return function_call(ret, to_function(this), argv[0], argc - 1, argv + 1);
}

int do_function_constructor(obj_t **ret, function_t *func, 
    obj_t *this, int argc, obj_t *argv[])
{
    scan_t *code = NULL;
    tstr_t body;
    tstr_list_t *params = NULL;
    call_t call;

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

	if (parse_function_param_list(&params, scanned_params))
	{
	    js_scan_uninit(scanned_params);
	    return throw_exception(ret, &S("Exception: Parse error"));
	}

	js_scan_uninit(scanned_params);
	tstr_free(&params_raw);
    }

    body = obj_get_str(argv[0]);
    code = js_scan_init(&body);
    call = call_evaluated_function;

Exit:
    *ret = function_new(params, code, global_env, call);
    return 0;
}
