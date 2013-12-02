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
#include "js/js_obj.h"
#include "js/js_event.h"
#include "platform/platform.h"

#define Stimers S("timers")
#define Stimer_id S("timer_id")

extern obj_t *meta_env;

void timers_uninit(void);
void timers_init(void);

static void timer_register(obj_t *timer_obj, int id)
{
    obj_t *timers;

    timers = obj_get_property(NULL, meta_env, &Stimers);
    array_push(timers, timer_obj);
    obj_put(timers);
}

static void timer_unregister_all(void)
{
    timers_uninit();
    timers_init();
}

static void timer_unregister(int id)
{
    obj_t *timers;
    array_iter_t iter;
    int idx = -1;

    timers = obj_get_property(NULL, meta_env, &Stimers);

    /* Lookup our timer */
    array_iter_init(&iter, timers, 0);
    while (array_iter_next(&iter))
    {
	int tid;

	if (obj_get_property_int(&tid, iter.obj, &Stimer_id))
	    continue;

	if (tid != id)
	    continue;

	idx = iter.k;
    }
    array_iter_uninit(&iter);

    /* XXX: should be deleting the entry entirely, not just
     * setting it to undefined
    */
    obj_set_int_property(timers, idx, UNDEF);
    obj_put(timers);
}

static int timer_function_call(obj_t **po, event_t *e)
{
    obj_t *this, *func;
    int ret;

    func = js_event_get_func(e);
    this = js_event_get_this(e);

    ret = function_call(po, this, 1, &func);

    obj_put(this);
    obj_put(func);
    return ret;
}

static void timeout_cb(event_t *e, int resource_id)
{
    obj_t *o;
    int tid;

    timer_function_call(&o, e);

    if (obj_get_property_int(&tid, js_event_obj(e), &Stimer_id))
	tp_err(("Weird, no timer id on timer obj\n"));
    else
	timer_unregister(tid);
    obj_put(o);
    js_event_free(e);
}

static void interval_cb(event_t *e, int resource_id)
{
    obj_t *o;

    if (timer_function_call(&o, e))
    {
	int tid;

	if (obj_get_property_int(&tid, js_event_obj(e), &Stimer_id))
	    tp_warn(("Weird, cannot find timer id\n"));
	else
	{
	    event_timer_del(tid);
	    timer_unregister(tid);
	}
	js_event_free(e);
    }

    obj_put(o);
}

int do_set_timeout(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    event_t *e;
    int ms, tid;

    tp_assert(argc == 3);

    e = js_event_new(argv[1], this, timeout_cb);

    ms = NUM_INT(to_num(argv[2]));

    tid = event_timer_set(ms, e);
    *ret = num_new_int(tid);
    obj_set_property(js_event_obj(e), Stimer_id, *ret);
    timer_register(js_event_obj(e), tid);
    return 0;
}
  
int do_set_interval(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    event_t *e;
    int ms, tid;

    tp_assert(argc == 3);

    e = js_event_new(argv[1], this, interval_cb);

    ms = NUM_INT(to_num(argv[2]));

    tid = event_timer_set_period(ms, e);
    *ret = num_new_int(tid);
    obj_set_property(js_event_obj(e), Stimer_id, *ret);
    timer_register(js_event_obj(e), tid);
    return 0;
}

static int do_clear_timer(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    tp_assert(argc == 1 || argc == 2);

    if (argc == 1)
    {
	event_timer_del_all();
	timer_unregister_all();
    }
    else
    {
	int id = NUM_INT(to_num(argv[1]));

	event_timer_del(id);
	timer_unregister(id);
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
    int ticks = platform.get_ticks_from_boot();

    /* Ticking every ms */
    *ret = num_new_fp((double)ticks / 1000);
    return 0;
}

int do_list_timers(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *timers = obj_get_property(NULL, meta_env, &Stimers);

    tp_out(("Timer list:\n%o\n", timers));

    obj_put(timers);
    return 0;
}

void timers_uninit(void)
{
}

void timers_init(void)
{
    _obj_set_property(meta_env, Stimers, array_new());
}
