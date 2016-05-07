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
#include "js/js_obj.h"
#include "js/js_gc.h"

extern void _obj_put_gc(obj_t *o);
extern void obj_free(obj_t *o);
extern obj_t *meta_env;

/* Local globals */
static obj_t *gc_del_list;
static u8 gc_mark_flag = OBJ_GC_MARK1;

static u8 gc_other_mark_flag(u8 mark_flag)
{
    return mark_flag == OBJ_GC_MARK1 ? OBJ_GC_MARK2 : OBJ_GC_MARK1;
}

static int gc_mark_cb(obj_t *o)
{
    if (o->flags & gc_mark_flag)
        return 1;

    o->flags |= gc_mark_flag;
    /* Clear the other mark flag for next run */
    o->flags &= ~gc_other_mark_flag(gc_mark_flag);
    return 0;
}

static void gc_sweep_cb(void *obj)
{
    obj_t *o = obj;

    if (!(o->flags & gc_mark_flag))
    {
        _obj_put_gc(o);
        /* This didn't free the object, only its properties.
         * It can't be freed at this point -- while iterating over the
         * object list.
         * Save it for later release.
         */
        o->next = gc_del_list;
        gc_del_list = o;
    }
}
static void gc_sweep(void)
{
    obj_t *delme;

    js_obj_foreach_alloced_obj(gc_sweep_cb);
    while ((delme = gc_del_list))
    {
        gc_del_list = gc_del_list->next;
        obj_free(delme);
    }
}

void __js_gc_run(int sweep_all)
{
    /* Keep two mark flags and alternate between them on each run. That way the
     * mark doesn't need to be cleared after each run.
     */
    gc_mark_flag = gc_other_mark_flag(gc_mark_flag);
    if (!sweep_all)
        obj_walk(meta_env, gc_mark_cb);
    gc_sweep();
}

void js_gc_run(void)
{
    __js_gc_run(0);
}
