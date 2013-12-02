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

#define Sevent_func S("event_func")
#define Sevent_this S("event_this")

typedef struct {
    event_t e; /* Must be first */
    obj_t *obj;
} js_event_t;

void js_event_free(event_t *e)
{
    obj_put(js_event_obj(e));
    tfree(e);
}

event_t *js_event_new(obj_t *func, obj_t *this, 
    void (*trigger)(event_t *e, int resource_id))
{
    js_event_t *jse = tmalloc_type(js_event_t);

    jse->obj = object_new();
    obj_set_property(jse->obj, Sevent_func, func);
    obj_set_property(jse->obj, Sevent_this, this);
    jse->e.trigger = trigger;
    jse->e.free = js_event_free;
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
