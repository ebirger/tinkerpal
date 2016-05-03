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
#include "js/js_utils.h"
#include "js/jsapi_decl.h"

int do_array_prototype_push(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int i;
    obj_t *obj = NULL;

    if (argc == 1)
    {
        *ret = num_new_int(array_length_get(this));
        return 0;
    }

    for (i = 1; i < argc; i++)
        obj = array_push(this, obj_get(argv[i]));
    *ret = obj_get(obj);
    return 0;
}

int do_array_prototype_pop(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    *ret = array_pop(this);
    return 0;
}

static int array_cb_call(obj_t **ret, function_t *cb, obj_t *cb_this, 
    obj_t *item, int index, obj_t *arr)
{
    int rc;
    obj_t *argv[4];

    argv[0] = (obj_t *)cb;
    argv[1] = item;
    argv[2] = num_new_int(index);
    argv[3] = arr;
    rc = function_call(ret, cb_this, 4, argv);
    obj_put(argv[2]);
    return rc;
}

int do_array_prototype_foreach(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    function_t *cb;
    obj_t *cb_this;
    array_iter_t iter;

    if (argc < 2)
        return js_invalid_args(ret);

    cb = to_function(argv[1]);
    cb_this = argc > 2 ? argv[2] : UNDEF;

    array_iter_init(&iter, this, 0);
    while (array_iter_next(&iter))
    {
        obj_t *dummy = NULL;

        array_cb_call(&dummy, cb, cb_this, iter.obj, iter.k, this);
        obj_put(dummy);
    }
    array_iter_uninit(&iter);
    *ret = UNDEF;
    return 0;
}

int do_array_prototype_indexof(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int start;
    obj_t *item;
    int is_eq = 0;
    array_iter_t iter;

    if (argc < 2)
        return js_invalid_args(ret);

    item = argv[1];
    start = argc > 2 ? NUM_INT(to_num(argv[2])) : 0;

    array_iter_init(&iter, this, 0);
    while (array_iter_next(&iter))
    {
        if (iter.k < start)
            continue;
        
        if ((is_eq = obj_eq(iter.obj, item)))
            break;
    }
    *ret = num_new_int(is_eq ? iter.k : -1);
    array_iter_uninit(&iter);
    return 0;
}

int do_array_prototype_join(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *sep;
    tstr_t comma = S(",");
    obj_t *o = NULL;
    array_iter_t iter;

    array_iter_init(&iter, this, ARRAY_ITER_FLAG_INCLUDE_EMPTY);
    if (iter.len == 0)
    {
        tstr_t empty = S("");
        *ret = string_new(empty);
        goto Exit;
    }

    sep = argc > 1 ? obj_get(argv[1]) : string_new(comma);

    while (array_iter_next(&iter))
    {
        obj_t *s;

        s = iter.obj ? obj_cast(iter.obj, STRING_CLASS) : string_new(S(""));

        if (o)
        {
            o = obj_do_op(TOK_PLUS, o, obj_get(sep));
            o = obj_do_op(TOK_PLUS, o, s);
        }
        else
            o = s;
    }
    obj_put(sep);
    *ret = o;

Exit:
    array_iter_uninit(&iter);
    return 0;
}

int do_array_prototype_map(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    function_t *cb;
    obj_t *cb_this, *new_arr;
    array_iter_t iter;
    int rc = 0;

    if (argc < 2)
        return js_invalid_args(ret);

    cb = to_function(argv[1]);
    cb_this = argc > 2 ? argv[2] : UNDEF;

    *ret = new_arr = array_new();

    array_iter_init(&iter, this, 0);
    if (iter.len == 0)
        goto Exit;

    while (array_iter_next(&iter))
    {
        obj_t *new_item = UNDEF;

        rc = array_cb_call(&new_item, cb, cb_this, iter.obj, iter.k, this);
        if (rc == COMPLETION_THROW)
        {
            obj_put(new_arr);
            *ret = new_item;
            goto Exit;
        }
        if (rc == COMPLETION_RETURN)
            rc = 0;

        _array_set_item(new_arr, iter.k, new_item);
    }

Exit:
    array_iter_uninit(&iter);
    return rc;
}

int do_array_prototype_filter(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    function_t *cb;
    obj_t *cb_this, *new_arr;
    array_iter_t iter;
    int rc = 0, new_arr_idx = 0;

    if (argc < 2)
        return js_invalid_args(ret);

    cb = to_function(argv[1]);
    cb_this = argc > 2 ? argv[2] : UNDEF;

    *ret = new_arr = array_new();

    array_iter_init(&iter, this, 0);
    if (iter.len == 0)
        goto Exit;

    while (array_iter_next(&iter))
    {
        obj_t *cb_rc = UNDEF;

        rc = array_cb_call(&cb_rc, cb, cb_this, iter.obj, iter.k, this);
        if (rc == COMPLETION_THROW)
        {
            obj_put(new_arr);
            *ret = cb_rc;
            goto Exit;
        }
        if (rc == COMPLETION_RETURN)
            rc = 0;

        if (obj_true(cb_rc))
            _array_set_item(new_arr, new_arr_idx++, obj_get(iter.obj));
        obj_put(cb_rc);
    }

Exit:
    array_iter_uninit(&iter);
    return rc;
}

int do_array_prototype_concat(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *new_arr;
    array_iter_t iter;
    int new_arr_idx = 0, n;

    *ret = new_arr = array_new();

    array_iter_init(&iter, this, 0);
    while (array_iter_next(&iter))
        _array_set_item(new_arr, new_arr_idx++, obj_get(iter.obj));
    array_iter_uninit(&iter);

    for (n = 1; n < argc; n++)
    {
        if (is_array(argv[n]))
        {
            array_iter_init(&iter, argv[n], 0);
            while (array_iter_next(&iter))
                _array_set_item(new_arr, new_arr_idx++, obj_get(iter.obj));
            array_iter_uninit(&iter);
        }
        else
            _array_set_item(new_arr, new_arr_idx++, obj_get(argv[n]));
    }
    return 0;
}

int do_array_prototype_slice(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *new_arr;
    array_iter_t iter;
    int rc = 0, start = 0, end = 0;

    if (argc >= 2)
    {
        if (argv[1] != UNDEF)
            start = obj_get_int(argv[1]);
        if (argc > 2 && argv[2] != UNDEF)
            end = obj_get_int(argv[2]);
    }

    *ret = new_arr = array_new();

    array_iter_init(&iter, this, 0);
    if (iter.len == 0)
        goto Exit;

    if (start < 0)
        start += iter.len;
    if (argc < 3)
        end = iter.len;
    else if (end < 0)
        end += iter.len;

    if (start > end)
        goto Exit;

    while (array_iter_next(&iter))
    {
        if (iter.k < start)
            continue;
        if (iter.k == end)
            break;

        array_push(new_arr, obj_get(iter.obj));
    }

Exit:
    array_iter_uninit(&iter);
    return rc;
}

static int array_sort_compare(obj_t **err, int *ge, obj_t *comparefn, obj_t *x,
    obj_t *y)
{
    extern obj_t *global_env;

    if (comparefn)
    {
        obj_t *argv[3], *retval = UNDEF;
        int rc;

        argv[0] = comparefn;
        argv[1] = x;
        argv[2] = y;
        rc = function_call(&retval, global_env, 3, argv);
        if (rc == COMPLETION_THROW)
        {
            *err = retval;
            return rc;
        }
        *ge = obj_get_int(retval) >= 0;
        obj_put(retval);
    }
    else
    {
        obj_t *strx = obj_cast(x, STRING_CLASS);
        obj_t *stry = obj_cast(y, STRING_CLASS);

        *ge = obj_true(obj_do_op(TOK_GE, strx, stry));
        *err = NULL;
    }
    return 0;
}

int do_array_prototype_sort(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int i, len;
    obj_t *comparefn = NULL;
    int rc = 0;

    if (argc > 2)
        return js_invalid_args(ret);

    if (argc == 2 && argv[1] != UNDEF)
        comparefn = argv[1];

    *ret = obj_get(this);

    if (!(len = array_length_get(this)))
        return 0;

    /* Insertion Sort */
    for (i = 1; i < len; i++)
    {
        obj_t *x;
        int j;

        x = array_lookup(this, i);
        if (!x)
            x = UNDEF;

        j = i;
        while (j > 0)
        {
            obj_t *min1, *excp = UNDEF;
            int ge;

            min1 = array_lookup(this, j - 1);
            /* non-existent properties are greater than undefined properties.
             * undefined properties are greater than any other value
             */
            if (!min1)
                min1 = UNDEF;
            if (min1 != UNDEF)
            {
                rc = array_sort_compare(&excp, &ge, comparefn, x, min1);
                if (rc == COMPLETION_THROW)
                {
                    obj_put(min1);
                    obj_put(x);
                    obj_put(*ret);
                    *ret = excp;
                    goto Exit;
                }

                if (ge > 0)
                {
                    obj_put(min1);
                    break;
                }
            }

            _array_set_item(this, j, min1);
            j--;
        }
        _array_set_item(this, j, x);
    }
Exit:
    return rc;
}

int do_array_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *a;

    a = array_new();

    if (argc == 2)
    {
        int len;

        if (!is_num(argv[1]))
        {
            array_push(a, obj_get(argv[1]));
            goto Exit;
        }

        len = obj_get_int(argv[1]);
        if (len < 0)
        {
            obj_put(a);
            return throw_exception(ret, &S("Exception: Invalid range"));
        }

        array_length_set(a, len);
    }
    else if (argc > 2)
    {
        do_array_prototype_push(ret, a, argc, argv);
        obj_put(*ret);
    }

Exit:
    *ret = a;
    return 0;
}
