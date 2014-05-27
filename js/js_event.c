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
#include "js/js_event.h"
#include "js/js_obj.h"

#define Sevents S("events")
#define Sevent_func S("event_func")
#define Sevent_this S("event_this")
#define Sevent_id S("event_id")

extern obj_t *meta_env;

static int g_js_event_id;

typedef struct {
    event_t e; /* Must be first */
    obj_t *obj;
} js_event_t;

static void js_event_unregister(int id)
{
    obj_t *events;

    events = obj_get_property(NULL, meta_env, &Sevents);
    /* XXX: we should really remove the object, not just set to undefined */
    obj_set_int_property(events, id, UNDEF);
    obj_put(events);
}

static void js_event_register(obj_t *obj, int id)
{
    obj_t *events;

    events = obj_get_property(NULL, meta_env, &Sevents);
    obj_set_int_property(events, id, obj);
    obj_put(events);
}

void js_event_free(event_t *e)
{
    obj_t *o = js_event_obj(e);
    int id = 0;

    obj_get_property_int(&id, o, &Sevent_id);
    js_event_unregister(id);
    obj_put(o);
    tfree(e);
}

event_t *js_event_new(obj_t *func, obj_t *this, 
    void (*trigger)(event_t *e, u32 resource_id, u32 timestamp))
{
    js_event_t *jse = tmalloc_type(js_event_t);
    int id = g_js_event_id++;

    jse->obj = object_new();
    obj_set_property(jse->obj, Sevent_func, func);
    obj_set_property(jse->obj, Sevent_this, this);
    obj_set_property_int(jse->obj, Sevent_id, id);
    jse->e.trigger = trigger;
    jse->e.free = js_event_free;
    js_event_register(jse->obj, id);
    return (event_t *)jse;
}

obj_t *js_event_obj(event_t *e)
{
    return ((js_event_t *)e)->obj;
}

obj_t *js_event_get_func(event_t *e)
{
    return obj_get_property(NULL, js_event_obj(e), &Sevent_func);
}

obj_t *js_event_get_this(event_t *e)
{
    return obj_get_property(NULL, js_event_obj(e), &Sevent_this);
}

void _js_event_gen_trigger(event_t *e, u32 id, obj_t *data_obj)
{
    obj_t *o, *this, *func, *argv[2];
    int argc = 0;

    argv[argc++] = func = js_event_get_func(e);
    if (data_obj)
	argv[argc++] = data_obj;
    this = js_event_get_this(e);

    function_call(&o, this, argc, argv);

    obj_put(func);
    obj_put(this);
    obj_put(o);
}

void js_event_gen_trigger(event_t *e, u32 id, u32 timestamp)
{
    _js_event_gen_trigger(e, id, NULL);
}

void js_event_uninit(void)
{
}

void js_event_init(void)
{
    _obj_set_property(meta_env, Sevents, object_new());
}
