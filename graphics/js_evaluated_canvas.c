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
#include "graphics/painter.desc"
#include "graphics/js_evaluated_canvas.h"
#include "js/js_obj.h"
#include "util/tp_misc.h"

#define SpixelDraw S(TpixelDraw)

typedef struct {
    canvas_t canvas;
    obj_t *obj;
} js_evaluated_canvas_t;

#define JS_CANVAS_FROM_CANVAS(c) container_of(c, js_evaluated_canvas_t, canvas);

void js_evaluated_canvas_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    js_evaluated_canvas_t *jscanvas = JS_CANVAS_FROM_CANVAS(c);
    obj_t *argv[4];
    obj_t *ret = UNDEF;

    argv[0] = obj_get_property(NULL, jscanvas->obj, &SpixelDraw);
    argv[1] = num_new_int(x);
    argv[2] = num_new_int(y);
    argv[3] = num_new_int(val);
    function_call(&ret, jscanvas->obj, 4, argv);
    obj_put(ret);
    obj_put(argv[0]);
    obj_put(argv[1]);
    obj_put(argv[2]);
    obj_put(argv[3]);
}

static const canvas_ops_t js_evaluated_canvas_ops = {
    .pixel_set = js_evaluated_canvas_pixel_set,
};

canvas_t *js_evaluated_canvas_new(obj_t *o)
{
    js_evaluated_canvas_t *jscanvas;

    /* XXX: assert that required functionality is provided by o */
    jscanvas = tmalloc_type(js_evaluated_canvas_t);
    jscanvas->canvas.ops = &js_evaluated_canvas_ops;
    jscanvas->obj = o;
    /* XXX: get width + height */
    canvas_register(&jscanvas->canvas);
    return &jscanvas->canvas;
}
