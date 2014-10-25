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
#include "mem/mem_cache.h"
#include "js/js_obj.h"
#include "js/js_utils.h"
#include "js/js_eval.h"
#include "js/js_compiler.h"
#include "platform/platform.h"
#include <math.h>

int do_eval(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    *ret = UNDEF;

    if (argc == 1)
        return 0;

    return js_eval_obj(ret, argv[1]);
}

int do_to_integer(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    num_t *n;
    int value = 0, sign;

    if (argc == 1)
        goto Exit;

    n = to_num(obj_cast(argv[1], NUM_CLASS));
    if (!NUM_IS_FP(n))
    {
        /* It is already an integer, just return our cast */
        *ret = &n->obj;
        return 0;
    }

    sign = NUM_FP(n) > 0 ? 1 : -1;
    value = (int)(floor(NUM_FP(n) * sign) * sign);
    obj_put(&n->obj);

Exit:
    *ret = num_new_int(value);
    return 0;
}

int do_is_nan(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *obj_num, *o;

    if (argc == 1)
        o = UNDEF;
    else
        o = argv[1];

    obj_num = obj_cast(o, NUM_CLASS);
    *ret = obj_num == NAN_OBJ ? TRUE : FALSE;
    obj_put(obj_num);
    return 0;
}

int do_assert(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *lv, *rv, *cond;
    int failed;

    if (argc != 3)
        return js_invalid_args(ret);

    lv = argv[1];
    rv = argv[2];
    /* Keep a reference for rv,lv so we can print them */
    cond = obj_do_op(TOK_IS_EQ, obj_get(rv), obj_get(lv));
    failed = !obj_true(cond);
    obj_put(cond);
    if (failed)
        tp_crit(("assertion failed %o != %o\n", lv, rv));

    *ret = UNDEF;
    return 0;
}

int do_assert_cond(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *cond;
    int failed;

    if (argc != 2)
        return js_invalid_args(ret);

    cond = argv[1];
    failed = !obj_true(cond);
    if (failed)
        tp_crit(("assertion failed: cond: %o\n", cond));

    *ret = UNDEF;
    return 0;
}

int do_assert_exception(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *o = UNDEF;
    int failed;

    if (argc != 2 || !is_function(argv[1]))
        return js_invalid_args(ret);

    failed = function_call(&o, this, argc - 1, argv + 1) != COMPLETION_THROW;
    tp_out(("output: %o\n", o));
    obj_put(o);
    if (failed)
        tp_crit(("Calling function did not result in exception\n"));

    *ret = UNDEF;
    return 0;
}

int do_dump_env(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    extern obj_t *global_env;

    if (argc != 1)
        return js_invalid_args(ret);

    tp_out(("%o\n", global_env));
    *ret = UNDEF;
    return 0;
}

int do_meminfo(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    tmalloc_stats();
    mem_cache_stats();
    platform_meminfo();
    return 0;
}

int do_describe(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    if (argc != 2)
        return js_invalid_args(ret);

    tp_out(("%D\n", argv[1]));
    *ret = UNDEF;
    return 0;
}

int do_compile(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int rc;

    if (argc != 2)
        return js_invalid_args(ret);

    *ret = obj_get(argv[1]);
    if ((rc = js_compile(ret)))
        return rc;

    return 0;
}
