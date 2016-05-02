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

int do_array_buffer_constructor(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int len = 0;

    if (argc > 1)
    {
        len = obj_get_int(argv[1]);
        if (len < 0)
            return throw_exception(ret, &S("Exception: Invalid range"));
    }

    *ret = array_buffer_new(len);
    return 0;
}

int do_array_buffer_view_subarray(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    array_buffer_view_t *v = to_array_buffer_view(this);
    int begin, end;

    if (argc == 1)
        begin = 0;
    else if ((begin = obj_get_int(argv[1])) < 0)
        begin += v->length;

    begin += v->offset;

    if (argc > 2)
    {
        if ((end = obj_get_int(argv[2])) < 0)
            end += v->length;
        end += v->offset;
    }
    else
        end = v->offset + v->length;

    if (begin < 0 || end < 0 || end < begin)
        end = begin = 0;

    *ret = array_buffer_view_new((obj_t *)v->array_buffer, v->flags, begin, 
        end - begin);
    return 0;
}

static void abv_cpy(array_buffer_view_t *dst, array_buffer_view_t *src)
{
    int idx;

    for (idx = 0; idx < src->length; idx++)
    {
        array_buffer_view_item_val_set(dst, idx,
            array_buffer_view_item_val_get(src, idx));
    }
}

static void arr_to_abv(array_buffer_view_t *dst, obj_t *arr)
{
    array_iter_t iter;

    array_iter_init(&iter, arr, 0);
    while (array_iter_next(&iter))
        array_buffer_view_item_val_set(dst, iter.k, obj_get_int(iter.obj));
    array_iter_uninit(&iter);
}

static int array_buffer_view_constructor(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[], unsigned short flags)
{
    obj_t *array_buffer, *orig_arr = NULL;
    int length, offset = 0;
    array_buffer_view_t *orig_abv = NULL;

    if (argc == 1)
    {
        length = 0;
        array_buffer = array_buffer_new(0);
    }
    else if (is_array_buffer(argv[1]))
    {
        array_buffer = obj_get(argv[1]);
        length = to_array_buffer(array_buffer)->value.len >> 
            (flags & ABV_SHIFT_MASK);
        if (argc > 2)
        {
            offset = obj_get_int(argv[2]);
            if (argc == 4)
                length = obj_get_int(argv[3]);
        }
    }
    else if (is_array_buffer_view(argv[1]))
    {
        orig_abv = (array_buffer_view_t *)argv[1];
        length = orig_abv->length;
        array_buffer = array_buffer_new(length << (flags & ABV_SHIFT_MASK));
    }
    else if (is_array(argv[1]))
    {
        orig_arr = argv[1];
        length = array_length_get(argv[1]);
        array_buffer = array_buffer_new(length << (flags & ABV_SHIFT_MASK));
    }
    else if (is_num(argv[1]))
    {
        length = obj_get_int(argv[1]);
        array_buffer = array_buffer_new(length << (flags & ABV_SHIFT_MASK));
    }
    else
    {
        length = 0;
        array_buffer = array_buffer_new(0);
    }

    *ret = array_buffer_view_new(array_buffer, flags, offset, length);

    if (orig_abv)
        abv_cpy(to_array_buffer_view(*ret), orig_abv);
    else if (orig_arr)
        arr_to_abv(to_array_buffer_view(*ret), orig_arr);

    obj_put(array_buffer);
    return 0;
}

#define TYPED_ARRAY_CONSTRUCTOR(name, flags) \
int do_##name##_constructor(obj_t **ret, obj_t *this, int argc, \
    obj_t *argv[]) \
{ \
    return array_buffer_view_constructor(ret, this, argc, argv, flags); \
}

TYPED_ARRAY_CONSTRUCTOR(int8array, ABV_SHIFT_8_BIT)
TYPED_ARRAY_CONSTRUCTOR(uint8array, ABV_SHIFT_8_BIT | ABV_FLAG_UNSIGNED)
TYPED_ARRAY_CONSTRUCTOR(int16array, ABV_SHIFT_16_BIT)
TYPED_ARRAY_CONSTRUCTOR(uint16array, ABV_SHIFT_16_BIT | ABV_FLAG_UNSIGNED)
TYPED_ARRAY_CONSTRUCTOR(int32array, ABV_SHIFT_32_BIT)
TYPED_ARRAY_CONSTRUCTOR(uint32array, ABV_SHIFT_32_BIT | ABV_FLAG_UNSIGNED)
