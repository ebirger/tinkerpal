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

int do_array_prototype_push(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    int i;
    obj_t *obj = NULL;

    tp_assert(argc > 0);
    for (i = 0; i < argc; i++)
	obj = array_push(this, obj_get(argv[i]));
    *ret = obj_get(obj);
    return 0;
}

int do_array_prototype_pop(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    tp_assert(argc == 0);
    *ret = array_pop(this);
    return 0;
}

static int array_cb_call(obj_t **ret, function_t *cb, obj_t *cb_this, 
    obj_t *item, int index, obj_t *arr)
{
    int rc;
    obj_t *argv[3];

    argv[0] = item;
    argv[1] = num_new_int(index);
    argv[2] = arr;
    rc = function_call(ret, cb, cb_this, 3, argv);
    obj_put(argv[1]);
    return rc;
}

int do_array_prototype_foreach(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    function_t *cb;
    obj_t *cb_this;
    array_iter_t iter;

    tp_assert(argc == 1 || argc == 2);

    cb = to_function(argv[0]);
    cb_this = argc == 2 ? argv[1] : UNDEF;

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

int do_array_prototype_indexof(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    int start;
    obj_t *item;
    int is_eq = 0;
    array_iter_t iter;

    tp_assert(argc == 1 || argc == 2);

    item = argv[0];
    start = argc == 2 ? NUM_INT(to_num(argv[1])) : 0;

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

int do_array_prototype_join(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    obj_t *sep;
    tstr_t comma = S(",");
    obj_t *o = NULL;
    array_iter_t iter;

    tp_assert(argc == 1 || argc == 0);

    array_iter_init(&iter, this, 0);
    if (iter.len == 0)
    {
	tstr_t empty = S("");
	*ret = string_new(empty);
	goto Exit;
    }

    sep = argc == 1 ? obj_get(argv[0]) : string_new(comma);

    while (array_iter_next(&iter))
    {
	obj_t *s;

	s = obj_cast(iter.obj, &string_class);

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

int do_array_prototype_map(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    function_t *cb;
    obj_t *cb_this, *new_arr;
    array_iter_t iter;

    tp_assert(argc == 1 || argc == 2);
    cb = to_function(argv[0]);
    cb_this = argc == 2 ? argv[1] : UNDEF;

    *ret = new_arr = array_new();

    array_iter_init(&iter, this, 0);
    if (iter.len == 0)
	goto Exit;

    while (array_iter_next(&iter))
    {
	obj_t *new_item;
	tstr_t kstr;

	kstr = int_to_tstr(iter.k);
	array_cb_call(&new_item, cb, cb_this, iter.obj, iter.k, this);
	_obj_set_property(new_arr, kstr, new_item);
	tstr_free(&kstr);
    }

Exit:
    array_iter_uninit(&iter);
    return 0;
}

int do_array_constructor(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    obj_t *a;

    a = array_new();

    if (argc == 1)
    {
	int len;

	if (!is_num(argv[0]))
	{
	    array_push(a, obj_get(argv[0]));
	    goto Exit;
	}

	len = obj_get_int(argv[0]);
	if (len < 0)
	{
	    obj_put(a);
	    return throw_exception(ret, &S("Exception: Invalid range"));
	}

	array_length_set(a, len);
    }
    else if (argc > 1)
    {
	do_array_prototype_push(ret, func, a, argc, argv);
	obj_put(*ret);
    }

Exit:
    *ret = a;
    return 0;
}
