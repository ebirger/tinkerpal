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
#include "js/js_obj.h"
#include "js/js_utils.h"
#include "js/js_event.h"
#include "js/jsapi_decl.h"
#include "drivers/serial/serial.h"
#include "drivers/serial/js_serial.h"

#define Sserial_id S("serial_id")

static int serial_read_fill_fn(void *ctx, char *buf, int size)
{
    u32 id = (u32)(int_ptr_t)ctx;

    return serial_read(id, buf, size);
}

static void serial_on_data_cb(event_t *e, u32 id, u64 timestamp)
{
    obj_t *o, *argv[2], *data_obj, *this, *func;
    tstr_t data;

    /* XXX: read as much as possible */
    tstr_init_alloc_data(&data, 30);
    data.len = tstr_fill(&data, 30, serial_read_fill_fn, (void *)(int_ptr_t)id);

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

int serial_obj_get_id(obj_t *o)
{
    int ret = -1;
    
    tp_assert(!obj_get_property_int(&ret, o, &Sserial_id));
    return ret;
}

int do_serial_enable(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    serial_enable(serial_obj_get_id(this), 1);
    return 0;
}

int do_serial_disable(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    serial_enable(serial_obj_get_id(this), 0);
    return 0;
}

int do_serial_on_data(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int event_id;

    if (argc > 2)
        return js_invalid_args(ret);

    if (argc == 1 || argv[1] == UNDEF)
    {
        event_watch_del_by_resource(serial_obj_get_id(this));
        *ret = UNDEF;
    }
    else if (argc == 2)
    {
        event_t *e;

        e = js_event_new(argv[1], this, serial_on_data_cb);

        /* XXX: if event is already set, it should be cleared */
        event_id = event_watch_set(serial_obj_get_id(this), e);
        *ret = num_new_int(event_id);
    }
    return 0;
}

static int do_serial_print_tstr_dump(void *ctx, char *buf, int len)
{
    obj_t *this = ctx;

    return serial_write(serial_obj_get_id(this), buf, len);
}

int do_serial_print(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    string_t *s;

    if (argc != 2)
        return js_invalid_args(ret);

    s = to_string(argv[1]);

    __tstr_dump(&s->value, 0, s->value.len, do_serial_print_tstr_dump, this);
    *ret = UNDEF;
    return 0;
}

int do_serial_write(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    if (argc != 2)
        return js_invalid_args(ret);

    *ret = UNDEF;

    if (is_string(argv[1]))
	return do_serial_print(ret, this, argc, argv);
    
    if (is_num(argv[1]))
    {
	int n = obj_get_int(argv[1]);
	char b;

	if (n < 0 || n > 255)
	    return throw_exception(ret, &S("Value must be in [0-255] range"));

	b = (char)n;
	serial_write(serial_obj_get_id(this), &b, 1);
	return 0;
    }
    
    if (is_array(argv[1]) || is_array_buffer_view(argv[1]))
    {
	array_iter_t iter;
	int rc = 0;

	array_iter_init(&iter, argv[1], 0);
	while (array_iter_next(&iter))
	{
	    obj_t *new_argv[2];

	    new_argv[0] = argv[0];
	    new_argv[1] = iter.obj;

	    if ((rc = do_serial_write(ret, this, 2, new_argv)))
		break;
	}
	array_iter_uninit(&iter);
	return rc;
    }

    /* Unknown parameter type */
    return js_invalid_args(ret);
}

int do_serial_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int id, params_set = 0;
    serial_params_t params = { .baud_rate = 115200 };

    if (argc != 2 && argc != 3)
        return js_invalid_args(ret);

    if (argc == 3)
    {
	obj_get_property_int(&params.baud_rate, argv[2], &S("baud_rate"));
        params_set = 1;
    }

    id = obj_get_int(argv[1]);
    serial_enable(id, 1);
    if (params_set)
    {
        if (serial_set_params(id, &params))
        {
            serial_enable(id, 0);
            return throw_exception(ret, &S("Failed setting serial params"));
        }
    }
    *ret = object_new();
    obj_inherit(*ret, argv[0]);
    obj_set_property_int(*ret, Sserial_id, id);
    return 0;
}
