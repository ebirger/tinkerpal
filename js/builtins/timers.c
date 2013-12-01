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
#include "platform/platform.h"

#define Stimers S("timers")
#define Stimer_func S("timer_func")
#define Stimer_this S("timer_this")
#define Stimer_id S("timer_id")

typedef struct {
    event_t e; /* Must be first */
    obj_t *timer_obj;
} delayed_work_t;

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

static void delayed_work_free(event_t *e)
{
    delayed_work_t *w = (delayed_work_t *)e;

    obj_put(w->timer_obj);
    tfree(w);
}

static delayed_work_t *delayed_work_new(obj_t *func, obj_t *this, 
    void (*expired)(event_t *e, int resource_id))
{
    delayed_work_t *w = tmalloc_type(delayed_work_t);

    w->timer_obj = object_new();
    obj_set_property(w->timer_obj, Stimer_func, func);
    obj_set_property(w->timer_obj, Stimer_this, this);
    w->e.trigger = expired;
    w->e.free = delayed_work_free;
    return w;
}

static int timer_function_call(obj_t **po, obj_t *timer_obj)
{
    obj_t *this, *func;
    int ret;

    func = obj_get_property(NULL, timer_obj, &Stimer_func);
    this = obj_get_property(NULL, timer_obj, &Stimer_this);

    ret = function_call(po, this, 1, &func);

    obj_put(this);
    obj_put(func);
    return ret;
}

static void timeout_cb(event_t *e, int resource_id)
{
    delayed_work_t *w = (delayed_work_t *)e;
    obj_t *o;
    int tid;

    timer_function_call(&o, w->timer_obj);

    if (obj_get_property_int(&tid, w->timer_obj, &Stimer_id))
	tp_err(("Weird, no timer id on timer obj\n"));
    else
	timer_unregister(tid);
    obj_put(o);
    delayed_work_free(e);
}

static void interval_cb(event_t *e, int resource_id)
{
    delayed_work_t *w = (delayed_work_t *)e;
    obj_t *o;

    if (timer_function_call(&o, w->timer_obj)) 
    {
	int tid;

	if (obj_get_property_int(&tid, w->timer_obj, &Stimer_id))
	    tp_warn(("Weird, cannot find timer id\n"));
	else
	{
	    event_timer_del(tid);
	    timer_unregister(tid);
	}
	delayed_work_free(e);
    }

    obj_put(o);
}

int do_set_timeout(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    delayed_work_t *w;
    int ms, tid;

    tp_assert(argc == 3);

    w = delayed_work_new(argv[1], this, timeout_cb);

    ms = NUM_INT(to_num(argv[2]));

    tid = event_timer_set(ms, &w->e);
    *ret = num_new_int(tid);
    obj_set_property(w->timer_obj, Stimer_id, *ret);
    timer_register(w->timer_obj, tid);
    return 0;
}
  
int do_set_interval(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    delayed_work_t *w;
    int ms, tid;

    tp_assert(argc == 3);

    w = delayed_work_new(argv[1], this, interval_cb);

    ms = NUM_INT(to_num(argv[2]));

    tid = event_timer_set_period(ms, &w->e);
    *ret = num_new_int(tid);
    obj_set_property(w->timer_obj, Stimer_id, *ret);
    timer_register(w->timer_obj, tid);
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
    obj_set_property(meta_env, Stimers, array_new());
}
