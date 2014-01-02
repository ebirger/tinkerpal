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
#include "util/event.h"
#include "util/debug.h"
#include "util/tp_misc.h"
#include "platform/platform.h"
#include "mem/tmalloc.h"
#include "js/js.h"

#define EVENT_FLAG_ON 0x0001
#define EVENT_FLAG_DELETED 0x0002
#define EVENT_FLAG_PERIODIC 0x0004

typedef struct event_internal_t {
    struct event_internal_t *next;
    event_t *e;
    u32 resource_id;
    u32 resource_mask;
    int event_id;
    int period;
    int expire;
    unsigned int flags;
} event_internal_t;

#define EVENT_IS_ON(e) ((e)->flags & EVENT_FLAG_ON)
#define EVENT_ON(e) bit_set((e)->flags, EVENT_FLAG_ON, 1)
#define EVENT_OFF(e) bit_set((e)->flags, EVENT_FLAG_ON, 0)
#define EVENT_IS_DELETED(e) ((e)->flags & EVENT_FLAG_DELETED)
#define EVENT_SET_DELETED(e) bit_set((e)->flags, EVENT_FLAG_DELETED, 1)
#define EVENT_IS_PERIODIC(e) ((e)->flags & EVENT_FLAG_PERIODIC)
#define EVENT_SET_PERIODIC(e) bit_set((e)->flags, EVENT_FLAG_PERIODIC, 1)

static event_internal_t *watches, *timers;
static int g_event_id = 0;

#define watches_foreach(e) for (e = watches; e; e = e->next)
#define timers_foreach(t) for (t = timers; t; t = t->next)

void event_timer_insert(event_internal_t *t, int ms)
{
    event_internal_t **iter;

    t->expire = platform_get_ticks_from_boot() + ms;

    /* Bug: we do not properly handle wrap-around */
    for (iter = &timers; *iter && (*iter)->expire < t->expire; 
	iter = &(*iter)->next);

    t->next = *iter;
    *iter = t;
}

static event_internal_t *_event_timer_set(int ms, event_t *e)
{
    event_internal_t *n = tmalloc_type(event_internal_t);

    n->e = e;
    n->event_id = g_event_id++;
    n->period = 0;
    n->flags = 0;
    event_timer_insert(n, ms);
    return n;
}

int event_timer_set(int ms, event_t *e)
{
    event_internal_t *n = _event_timer_set(ms, e);
    return n->event_id;
}

int event_timer_set_period(int ms, event_t *e)
{
    event_internal_t *n = _event_timer_set(ms, e);
    n->period = ms;
    EVENT_SET_PERIODIC(n);
    return n->event_id;
}

void event_timer_del(int event_id)
{
    event_internal_t *t;

    timers_foreach(t)
    {
	if (t->event_id == event_id)
	    EVENT_SET_DELETED(t);
    }
}

void event_timer_del_all(void)
{
    event_internal_t *t;

    timers_foreach(t)
	EVENT_SET_DELETED(t);
}

static void timeout_process(void)
{
    event_internal_t **iter = &timers, *t;

    while ((t = *iter))
    {
	event_t *e = t->e;

	if (EVENT_IS_DELETED(t))
	{
	    iter = &(*iter)->next;
	    continue;
	}

	if (platform_get_ticks_from_boot() < t->expire)
	    break;

	if (EVENT_IS_PERIODIC(t))
	{
	    /* Remove from list and re-insert at the proper time */
	    *iter = (*iter)->next;
	    event_timer_insert(t, t->period);
	}
	else
	    EVENT_SET_DELETED(t);

	e->trigger(e, 0 /* dummy */);

	/* Go back to the start. We don't know what happend to the list
	 * while we ran the cb.
	 */
	iter = &timers;
    }
}

static void get_next_timeout(int *timeout)
{
    if (!timers)
    {
	*timeout = 0;
	return;
    }
    *timeout = timers->expire - platform_get_ticks_from_boot();
    tp_debug(("Next timeout: %d ms\n", *timeout));
}

void event_watch_trigger(u32 resource_id)
{
    event_internal_t *e;

    watches_foreach(e)
    {
	if ((e->resource_id ^ resource_id) & e->resource_mask)
	    continue;

	EVENT_ON(e);
    }
}

int _event_watch_set(u32 resource_id, u32 resource_mask, event_t *e)
{
    event_internal_t *n;
    
    n = tmalloc_type(event_internal_t);

    n->event_id = g_event_id++;
    n->resource_id = resource_id;
    n->resource_mask = resource_mask;
    n->e = e;
    n->next = watches;
    n->flags = 0;
    watches = n;
    return n->event_id;
}

void event_watch_del(int event_id)
{
    event_internal_t *e;

    watches_foreach(e)
    {
	if (e->event_id == event_id)
	    EVENT_SET_DELETED(e);
    }
}

void event_watch_del_by_resource(u32 resource_id)
{
    event_internal_t *e;

    watches_foreach(e)
    {
	if (e->resource_id == resource_id)
	    EVENT_SET_DELETED(e);
    }
}

void event_watch_del_all(void)
{
    event_internal_t *e;

    watches_foreach(e)
	EVENT_SET_DELETED(e);
}

void event_purge_deleted(event_internal_t **events)
{
    event_internal_t *e;

    while ((e = *events))
    {
	if (EVENT_IS_DELETED(e))
	{
	    *events = (*events)->next;

	    if (e->e->free)
		e->e->free(e->e);
	    tfree(e);
	}
	else
	    events = &(*events)->next;
    }
}

static int watches_process(void)
{
    event_internal_t *e;
    int more = 0;

    watches_foreach(e)
    {
	if (EVENT_IS_DELETED(e) || !(EVENT_IS_ON(e)))
	    continue;

	EVENT_OFF(e);
	e->e->trigger(e->e, e->resource_id);
	/* trigger may have triggered new watches */
	more = 1;
    }

    return more;
}

void event_loop(void)
{
    while (watches || timers) 
    {
	int timeout, more_watches;

	more_watches = watches_process();

	get_next_timeout(&timeout);

	if (timeout >= 0 && !more_watches)
	    platform.select(timeout);

	timeout_process();

	event_purge_deleted(&timers);
	event_purge_deleted(&watches);
    }

    event_timer_del_all();
    event_watch_del_all();
    event_purge_deleted(&timers);
    event_purge_deleted(&watches);
}
