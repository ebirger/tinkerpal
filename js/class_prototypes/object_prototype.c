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
#include "js/js_emitter.h"
#include "js/js_utils.h"

int do_object_prototype_to_string(obj_t **ret, obj_t *this, int argc, 
    obj_t *argv[])
{
    if (argc == 2 && is_num(this))
    {
        num_t *n = to_num(this);
        int radix;

        if (NUM_IS_FP((n = to_num(this))))
            return throw_exception(ret, &S("Not supported yet"));

        radix = obj_get_int(argv[1]);
        *ret = string_new(_int_to_tstr(NUM_INT(n), radix));
    }
    else
        *ret = obj_cast(this, STRING_CLASS);
    return 0;
}

int do_object_prototype_on(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    tstr_t event;

    if (argc != 3 || !is_function(argv[2]))
        return js_invalid_args(ret);

    event = obj_get_str(argv[1]);

    js_obj_on(this, event, argv[2]);

    tstr_free(&event);

    *ret = UNDEF;
    return 0;
}

int do_object_prototype_emit(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    tstr_t event;
    obj_t *tmp;

    if (argc < 2)
        return js_invalid_args(ret);

    event = obj_get_str(argv[1]);

    tmp = argv[1];
    js_obj_emit(this, event, argc - 1, argv + 1);
    argv[1] = tmp;

    tstr_free(&event);
    *ret = UNDEF;
    return 0;
}

int do_object_prototype_remove_all_listeners(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    if (argc == 1)
        js_obj_remove_all_listeners(this);
    else
    {
        tstr_t event = obj_get_str(argv[1]);

        js_obj_remove_listeners(this, event);
        tstr_free(&event);
    }

    *ret = UNDEF;
    return 0;
}

int do_object_prototype_listeners(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    tstr_t event;

    if (argc != 2)
        return js_invalid_args(ret);

    event = obj_get_str(argv[1]);

    *ret = js_obj_listeners(this, event);

    tstr_free(&event);
    return 0;
}

int do_object_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    *ret = object_new();
    return 0;
}
