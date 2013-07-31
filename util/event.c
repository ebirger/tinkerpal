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
#include <stdio.h> /* NULL */
#include "util/tmalloc.h"
#include "util/event.h"
#include "util/debug.h"
#include "platform/platform.h"
#include "js/js.h"

/* XXX: consolidate with event_timer_internal_t */
typedef struct event_watch_internal_t {
    struct event_watch_internal_t *next;
    event_watch_t *ew;
    int resource_id;
    int watch_id;
    int is_on;
    int deleted;
} event_watch_internal_t;

typedef struct event_timer_internal_t {
    struct event_timer_internal_t *next;
    event_timer_t *et;
    int timer_id;
    int period;
    int expire;
} event_timer_internal_t;

static event_watch_internal_t *watches;
static event_timer_internal_t *timer_list = NULL;
static int g_timer_id = 0, g_watch_id = 0;

#define watches_foreach(e) for (e = watches; e; e = e->next)

void event_timer_insert(event_timer_internal_t *t, int ms)
{
    event_timer_internal_t **iter;

    t->expire = platform.get_ticks_from_boot() + ms;

    /* Bug: we do not properly handle wrap-around */
    for (iter = &timer_list; *iter && (*iter)->expire < t->expire; 
	iter = &(*iter)->next);

    t->next = *iter;
    *iter = t;
}

static event_timer_internal_t *_event_timer_set(int ms, int period, 
    event_timer_t *et)
{
    event_timer_internal_t *n = tmalloc_type(event_timer_internal_t);

    n->et = et;
    n->period = period;
    n->timer_id = g_timer_id++;
    event_timer_insert(n, ms);
    return n;
}

int event_timer_set(int ms, event_timer_t *et)
{
    event_timer_internal_t *n = _event_timer_set(ms, 0, et);
    return n->timer_id;
}

int event_timer_set_period(int ms, event_timer_t *et)
{
    event_timer_internal_t *n = _event_timer_set(ms, ms, et);
    return n->timer_id;
}

void event_timer_del(int timer_id)
{
    event_timer_internal_t **iter, *t;
   
    for (iter = &timer_list; *iter && (*iter)->timer_id != timer_id; 
	iter = &(*iter)->next);
    if (!(t = *iter))
	return;
    
    *iter = (*iter)->next;
    if (t->et->free)
	t->et->free(t->et);
    tfree(t);
}

void event_timer_del_all(void)
{
    event_timer_internal_t *t;

    while ((t = timer_list))
    {
	timer_list = timer_list->next;
	if (t->et->free)
	    t->et->free(t->et);
	tfree(t);
    }
}

static void timeout_process(void)
{
    event_timer_internal_t **iter = &timer_list;

    while (*iter)
    {
	event_timer_internal_t *t = *iter;
	event_timer_t *et = t->et;

	if (platform.get_ticks_from_boot() < t->expire)
	    break;

	/* Remove current timer from list */
	*iter = (*iter)->next;

	if (t->period)
	    event_timer_insert(t, t->period);
	else
	    tfree(t);

	et->expired(et);

	/* Go back to the start. We don't know what happend to the list
	 * while we ran the cb.
	 */
	iter = &timer_list;
    }
}

static void get_next_timeout(int *timeout)
{
    if (!timer_list)
    {
	*timeout = 0;
	return;
    }
    *timeout = timer_list->expire - platform.get_ticks_from_boot();
    tp_debug(("Next timeout: %d ms\n", *timeout));
}

static event_watch_internal_t *watch_lookup(int resource_id)
{
    event_watch_internal_t *e;

    watches_foreach(e)
    {
	if (e->resource_id == resource_id)
	    return e;
    }
    return NULL;
}

static void event_mark_on(int resource_id)
{
    event_watch_internal_t *e;

    if (!(e = watch_lookup(resource_id)))
	return;

    e->is_on = 1;
}

static int event_is_active(int resource_id)
{
    event_watch_internal_t *e;

    if (!(e = watch_lookup(resource_id)))
	return 0;

    return e->resource_id != -1;
}

int event_watch_set(int resource_id, event_watch_t *ew)
{
    event_watch_internal_t *n;
    
    n = tmalloc_type(event_watch_internal_t);

    n->watch_id = g_watch_id++;
    n->resource_id = resource_id;
    n->ew = ew;
    n->is_on = 0;
    n->next = watches;
    n->deleted = 0;
    watches = n;
    return n->watch_id;
}

void event_watch_del(int watch_id)
{
    event_watch_internal_t *e;

    watches_foreach(e)
    {
	if (e->watch_id == watch_id)
	    e->deleted = 1;
    }
}

void event_watch_del_all(void)
{
    event_watch_internal_t *e;

    watches_foreach(e)
	e->deleted = 1;
}

void event_purge_deleted(void)
{
    event_watch_internal_t **iter = &watches, *e;

    while ((e = *iter))
    {
	if (e->deleted)
	{
	    *iter = (*iter)->next;

	    if (e->ew->free)
		e->ew->free(e->ew);
	    tfree(e);
	}
	else
	    iter = &(*iter)->next;
    }
}

static void watches_process(void)
{
    event_watch_internal_t *e;

    watches_foreach(e)
    {
	if (!e->is_on)
	    continue;

	e->ew->watch_event(e->ew, e->resource_id);
	e->is_on = 0;
    }
}

void event_loop(void)
{
    while (watches || timer_list) 
    {
	int timeout, rc = 0;

	get_next_timeout(&timeout);

	if (timeout >= 0)
	    rc = platform.select(timeout, event_is_active, event_mark_on);

	if (rc)
	    watches_process();
	else
	    timeout_process();

	event_purge_deleted();
    }

    event_timer_del_all();
    event_watch_del_all();
    event_purge_deleted();
}
