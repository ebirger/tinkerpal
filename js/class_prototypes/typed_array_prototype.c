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

int do_array_buffer_constructor(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    int len;

    tp_assert(argc == 2);

    len = obj_get_int(argv[1]);
    if (len < 0)
	return throw_exception(ret, &S("Exception: Invalid range"));

    *ret = array_buffer_new(len);
    return 0;
}

static int array_buffer_view_constructor(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[], unsigned short flags)
{
    *ret = array_buffer_view_new(argv[1], flags);
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
