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
#include "util/event.h"
#include "util/debug.h"
#include "main/console.h"
#include "js/js_obj.h"
#include "js/js_utils.h"
#include "graphics/graphics.h"
#include "graphics/js_canvas.h"
#include "graphics/js_evaluated_canvas.h"

#define Scanvas S("canvas")

static canvas_t *graphics_obj_get_canvas(obj_t *gr)
{
    obj_t *o;

    if (!(o = obj_get_property(NULL, gr, &Scanvas)))
        return NULL;

    return canvas_obj_get_canvas(o);
}

int do_graphics_rect_draw(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int x, y, w, h, color;
    canvas_t *c;

    if (argc != 6)
        return js_invalid_args(ret);

    if (!(c = graphics_obj_get_canvas(this)))
        return js_invalid_args(ret);

    x = obj_get_int(argv[1]);
    y = obj_get_int(argv[2]);
    w = obj_get_int(argv[3]);
    h = obj_get_int(argv[4]);
    color = obj_get_int(argv[5]);

    rect_draw(c, x, y, w, h, color);
    return 0;
}

int do_graphics_rect_fill(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int x, y, w, h, color;
    canvas_t *c;

    if (argc != 6)
        return js_invalid_args(ret);

    if (!(c = graphics_obj_get_canvas(this)))
        return js_invalid_args(ret);

    x = obj_get_int(argv[1]);
    y = obj_get_int(argv[2]);
    w = obj_get_int(argv[3]);
    h = obj_get_int(argv[4]);
    color = obj_get_int(argv[5]);

    rect_fill(c, x, y, w, h, color);
    return 0;
}

int do_graphics_round_rect_draw(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    int x, y, w, h, r, color;
    canvas_t *c;

    if (argc != 7)
        return js_invalid_args(ret);

    if (!(c = graphics_obj_get_canvas(this)))
        return js_invalid_args(ret);

    x = obj_get_int(argv[1]);
    y = obj_get_int(argv[2]);
    w = obj_get_int(argv[3]);
    h = obj_get_int(argv[4]);
    r = obj_get_int(argv[5]);
    color = obj_get_int(argv[6]);

    round_rect_draw(c, x, y, w, h, r, ROUND_RECT_TYPE_REGULAR, color);
    return 0;
}

int do_graphics_line_draw(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int x0, y0, x1, y1, color;
    canvas_t *c;

    if (argc != 6)
        return js_invalid_args(ret);

    if (!(c = graphics_obj_get_canvas(this)))
        return js_invalid_args(ret);

    x0 = obj_get_int(argv[1]);
    y0 = obj_get_int(argv[2]);
    x1 = obj_get_int(argv[3]);
    y1 = obj_get_int(argv[4]);
    color = obj_get_int(argv[5]);

    line_draw(c, x0, y0, x1, y1, color);
    return 0;
}

int do_graphics_circle_draw(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int x, y, radius, color;
    canvas_t *c;

    if (argc != 5)
        return js_invalid_args(ret);

    if (!(c = graphics_obj_get_canvas(this)))
        return js_invalid_args(ret);

    x = obj_get_int(argv[1]);
    y = obj_get_int(argv[2]);
    radius = obj_get_int(argv[3]);
    color = obj_get_int(argv[4]);

    circle_draw(c, x, y, radius, color);
    return 0;
}

int do_graphics_circle_fill(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int x, y, radius, color;
    canvas_t *c;

    if (argc != 5)
        return js_invalid_args(ret);

    if (!(c = graphics_obj_get_canvas(this)))
        return js_invalid_args(ret);

    x = obj_get_int(argv[1]);
    y = obj_get_int(argv[2]);
    radius = obj_get_int(argv[3]);
    color = obj_get_int(argv[4]);

    circle_fill(c, x, y, radius, color);
    return 0;
}

int do_graphics_string_draw(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int x, y, color;
    string_t *s;
    canvas_t *c;

    if (argc != 5)
        return js_invalid_args(ret);
    
    if (!(c = graphics_obj_get_canvas(this)))
        return js_invalid_args(ret);

    x = obj_get_int(argv[1]);
    y = obj_get_int(argv[2]);
    s = to_string(argv[3]);
    color = obj_get_int(argv[4]);

    string_draw(c, x, y, &s->value, color);
    return 0;
}

int do_graphics_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    canvas_t *canvas;
    obj_t *o, *canvas_obj;

    if (argc != 2)
        return js_invalid_args(ret);

    o = canvas_obj = argv[1];

    if (!(canvas = canvas_obj_get_canvas(o)))
    {
        obj_t *argv = UNDEF;

        /* No canvas found. Try to create an evaluated canvas */
        if (!(canvas = js_evaluated_canvas_new(o)))
            return js_invalid_args(ret);

        canvas_obj_constructor(canvas, &canvas_obj, NULL, 1, &argv);
    }
    else
        obj_get(canvas_obj);


    *ret = object_new();
    obj_inherit(*ret, argv[0]);
    _obj_set_property(*ret, Scanvas, canvas_obj);
    return 0;
}
