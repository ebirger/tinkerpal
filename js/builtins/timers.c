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
#include "platform/platform.h"

#define Stimer_id S("timer_id")

static void interval_cb(event_t *e, u32 resource_id)
{
    obj_t *o, *this, *func;

    func = js_event_get_func(e);
    this = js_event_get_this(e);

    if (function_call(&o, this, 1, &func))
    {
	int tid = 0;

	obj_get_property_int(&tid, js_event_obj(e), &Stimer_id);
	event_timer_del(tid);
    }

    obj_put(this);
    obj_put(func);
    obj_put(o);
}

int do_set_timeout(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    event_t *e;
    int ms, tid;

    if (argc != 3)
	return js_invalid_args(ret);

    e = js_event_new(argv[1], this, js_event_gen_trigger);

    ms = NUM_INT(to_num(argv[2]));

    tid = event_timer_set(ms, e);
    *ret = num_new_int(tid);
    obj_set_property(js_event_obj(e), Stimer_id, *ret);
    return 0;
}
  
int do_set_interval(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    event_t *e;
    int ms, tid;

    if (argc != 3)
	return js_invalid_args(ret);

    e = js_event_new(argv[1], this, interval_cb);

    ms = NUM_INT(to_num(argv[2]));

    tid = event_timer_set_period(ms, e);
    *ret = num_new_int(tid);
    obj_set_property(js_event_obj(e), Stimer_id, *ret);
    return 0;
}

static int do_clear_timer(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    if (argc != 1 && argc != 2)
	return js_invalid_args(ret);

    if (argc == 1)
	event_timer_del_all();
    else
    {
	int id = NUM_INT(to_num(argv[1]));

	event_timer_del(id);
    }
    return 0;
}

int do_clear_interval(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    return do_clear_timer(ret, this, argc, argv);
}

int do_clear_timeout(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    return do_clear_timer(ret, this, argc, argv);
}

int do_get_time(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int ticks = platform_get_ticks_from_boot();

    /* Ticking every ms */
    *ret = num_new_fp((double)ticks / 1000);
    return 0;
}
