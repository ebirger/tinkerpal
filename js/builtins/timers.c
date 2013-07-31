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

typedef struct {
    event_timer_t et; /* Must be first */
    obj_t *func;
    obj_t *this;
    int timer_id;
} delayed_work_t;

static void delayed_work_free(event_timer_t *et)
{
    delayed_work_t *w = (delayed_work_t *)et;

    obj_put(w->func);
    obj_put(w->this);
    tfree(w);
}

static delayed_work_t *delayed_work_new(obj_t *func, obj_t *this, 
    void (*expired)(event_timer_t *et))
{
    delayed_work_t *w = tmalloc_type(delayed_work_t);

    w->func = obj_get(func);
    w->this = obj_get(this);
    w->et.expired = expired;
    w->et.free = delayed_work_free;
    return w;
}

static void timeout_cb(event_timer_t *et)
{
    delayed_work_t *w = (delayed_work_t *)et;
    obj_t *o;

    function_call(&o, to_function(w->func), w->this, 0, NULL);

    obj_put(o);
    delayed_work_free(et);
}

static void interval_cb(event_timer_t *et)
{
    delayed_work_t *w = (delayed_work_t *)et;
    obj_t *o;

    if (function_call(&o, to_function(w->func), w->this, 0, NULL)) 
    {
	event_timer_del(w->timer_id);
	delayed_work_free(et);
    }

    obj_put(o);
}

int do_set_timeout(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    delayed_work_t *w;
    int ms, tid;

    tp_assert(argc == 2);

    w = delayed_work_new(argv[0], this, timeout_cb);

    ms = NUM_INT(to_num(argv[1]));

    tid = event_timer_set(ms, &w->et);
    *ret = num_new_int(tid);
    return 0;
}
  
int do_set_interval(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    delayed_work_t *w;
    int ms, tid;

    tp_assert(argc == 2);

    w = delayed_work_new(argv[0], this, interval_cb);

    ms = NUM_INT(to_num(argv[1]));

    tid = event_timer_set_period(ms, &w->et);
    w->timer_id = tid;
    *ret = num_new_int(w->timer_id);
    return 0;
}

static int do_clear_timer(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    tp_assert(argc == 0 || argc == 1);

    if (argc == 0)
	event_timer_del_all();
    else
    {
	int id = NUM_INT(to_num(argv[0]));

	event_timer_del(id);
    }
    return 0;
}

int do_clear_interval(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    return do_clear_timer(ret, func, this, argc, argv);
}

int do_clear_timeout(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    return do_clear_timer(ret, func, this, argc, argv);
}

int do_get_time(obj_t **ret, function_t *func, obj_t *this,
    int argc, obj_t *argv[])
{
    int ticks = platform.get_ticks_from_boot();

    /* Ticking every ms */
    *ret = num_new_fp((double)ticks / 1000);
    return 0;
}
