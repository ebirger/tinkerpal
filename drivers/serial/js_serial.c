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
#include "util/event.h"
#include "util/debug.h"
#include "main/console.h"
#include "js/js_obj.h"
#include "js/js_event.h"
#include "drivers/serial/serial.h"

#define Sserial_id S("serial_id")

static void serial_on_data_cb(event_t *e, int id)
{
    obj_t *o, *argv[2], *data_obj, *this, *func;
    tstr_t data;

    /* XXX: read as much as possible */
    tstr_alloc(&data, 30);
    data.len = serial_read(id, TPTR(&data), 30);

    data_obj = object_new();
    obj_set_property_str(data_obj, S("data"), data);

    argv[0] = func = js_event_get_func(e);
    argv[1] = data_obj;
    this = js_event_get_this(e);

    function_call(&o, this, 2, argv);

    obj_put(func);
    obj_put(this);
    obj_put(o);
    obj_put(data_obj);
}

static int get_serial_id(obj_t *o)
{
    int ret = -1;
    
    tp_assert(!obj_get_property_int(&ret, o, &Sserial_id));
    return ret;
}

int do_serial_enable(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    serial_enable(get_serial_id(this), 1);
    return 0;
}

int do_serial_disable(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    serial_enable(get_serial_id(this), 0);
    return 0;
}

int do_serial_on_data(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    event_t *e;
    int event_id;

    tp_assert(argc == 2);

    e = js_event_new(argv[1], this, serial_on_data_cb);

    /* XXX: if event is already set, it should be cleared */
    event_id = event_watch_set(get_serial_id(this), e);
    *ret = num_new_int(event_id);
    return 0;
}

int do_serial_print(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    string_t *s;

    tp_assert(argc == 2);

    s = to_string(argv[1]);

    serial_write(get_serial_id(this), TPTR(&s->value), s->value.len);
    *ret = UNDEF;
    return 0;
}

int do_serial_set_console(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    console_set_id(get_serial_id(this));
    *ret = UNDEF;
    return 0;
}

int do_serial_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int id;

    tp_assert(argc == 2);

    id = obj_get_int(argv[1]);
    *ret = object_new();
    obj_inherit(*ret, argv[0]);
    obj_set_property_int(*ret, Sserial_id, id);
    serial_enable(id, 1);
    return 0;
}
