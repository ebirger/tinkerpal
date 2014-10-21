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
#include "js/js_emitter.h"

#define Son_events INTERNAL_S("___on_events___")
static obj_t *obj_get_on_events_root(obj_t *o)
{
    obj_t *root;

    root = obj_get_own_property(NULL, o, &Son_events);
    if (!root || root == UNDEF)
    {
        root = object_new();
        obj_set_property(o, Son_events, root);
    }
    return root;
}

void js_obj_on(obj_t *o, tstr_t event, obj_t *func)
{
    obj_t *listeners, *root;

    root = obj_get_on_events_root(o);
    listeners = obj_get_own_property(NULL, root, &event);
    /* XXX: we are accounting for possible dangling listener key */
    if (!listeners || listeners == UNDEF)
    {
        listeners = array_new();
        obj_set_property(root, event, listeners);
    }

    array_push(listeners, obj_get(func));

    obj_put(listeners);
    obj_put(root);
}

void js_obj_emit(obj_t *o, tstr_t event, int argc, obj_t *argv[])
{
    obj_t *listeners, *root;
    array_iter_t iter;

    root = obj_get_on_events_root(o);
    listeners = obj_get_own_property(NULL, root, &event);
    if (!listeners)
        goto Exit;

    array_iter_init(&iter, listeners, 0);
    while (array_iter_next(&iter))
    {
        obj_t *ret;

        argv[0] = iter.obj;
        function_call(&ret, o, argc, argv);
        obj_put(ret);
    }
    array_iter_uninit(&iter);
    obj_put(listeners);
Exit:
    obj_put(root);
}

obj_t *js_obj_listeners(obj_t *o, tstr_t event)
{
    obj_t *listeners, *root;

    root = obj_get_on_events_root(o);
    listeners = obj_get_own_property(NULL, root, &event);
    obj_put(root);
    return listeners ? : UNDEF;
}

void js_obj_remove_listeners(obj_t *o, tstr_t event)
{
    obj_t **lval, *listeners, *root;

    root = obj_get_on_events_root(o);
    listeners = obj_get_own_property(&lval, root, &event);
    if (!listeners)
        goto Exit;

    *lval = UNDEF;
    obj_put(listeners); /* Remove our reference */
    obj_put(listeners); /* Remove the object's reference */

    /* XXX: there is a dangling container left */
Exit:
    obj_put(root);
}

void js_obj_remove_all_listeners(obj_t *o)
{
    obj_t *root, **lval;

    root = obj_get_own_property(&lval, o, &Son_events);
    if (!root)
        return;

    *lval = UNDEF;
    obj_put(root); /* Remove our reference */
    obj_put(root); /* Remove the object's reference */
}
