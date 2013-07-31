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
#include "util/tmalloc.h"
#include "util/event.h"
#include "util/debug.h"
#include "main/console.h"
#include "js/js_obj.h"
#include "drivers/serial/serial.h"

#define Sserial_id S("serial_id")

typedef struct {
    event_watch_t ew; /* Must be first */
    int id;
    obj_t *func;
    obj_t *this;
} serial_work_t;

static void serial_work_free(event_watch_t *ew)
{
    serial_work_t *w = (serial_work_t *)ew;
    obj_put(w->func);
    obj_put(w->this);
    tfree(w);
}

static serial_work_t *serial_work_new(int id, obj_t *func, obj_t *this, 
    void (*watch_event)(event_watch_t *ew, int resource_id))
{
    serial_work_t *w = tmalloc_type(serial_work_t);

    w->ew.watch_event = watch_event;
    w->ew.free = serial_work_free;
    w->id = id;
    w->func = obj_get(func);
    w->this = obj_get(this);
    return w;
}

static void serial_on_data_cb(event_watch_t *ew, int id)
{
    serial_work_t *w = (serial_work_t *)ew;
    obj_t *o, *data_obj;
    tstr_t data;

    /* XXX: read as much as possible */
    data.value = tmalloc(30, "serial_data");
    data.len = serial_read(w->id, data.value, 30);
    TSTR_SET_ALLOCATED(&data);

    data_obj = object_new();
    obj_set_property_str(data_obj, S("data"), data);

    function_call(&o, to_function(w->func), w->this, 1, &data_obj);

    obj_put(o);
    obj_put(data_obj);
}

static int get_serial_id(obj_t *o)
{
    int ret = -1;
    
    tp_assert(!obj_get_property_int(&ret, o, Sserial_id));
    return ret;
}

int do_serial_enable(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    serial_enable(get_serial_id(this), 1);
    return 0;
}

int do_serial_disable(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    serial_enable(get_serial_id(this), 0);
    return 0;
}

int do_serial_on_data(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    serial_work_t *w;
    int event_id;

    tp_assert(argc == 1);

    w = serial_work_new(get_serial_id(this), argv[0], this, serial_on_data_cb);

    /* XXX: if event is already set, it should be cleared */
    event_id = event_watch_set(w->id, &w->ew);
    *ret = num_new_int(event_id);
    return 0;
}

int do_serial_print(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    string_t *s;

    tp_assert(argc == 1);

    s = to_string(argv[0]);

    serial_write(get_serial_id(this), s->value.value, s->value.len);
    *ret = UNDEF;
    return 0;
}

int do_serial_set_console(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    console_set_id(get_serial_id(this));
    *ret = UNDEF;
    return 0;
}

int do_serial_constructor(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    int id;

    tp_assert(argc == 1);

    id = obj_get_int(argv[0]);
    *ret = object_new();
    (*ret)->prototype = obj_get(func->obj.prototype);
    obj_set_property_int(*ret, Sserial_id, id);
    serial_enable(id, 1);
    return 0;
}
