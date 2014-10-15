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
#include "graphics/canvas.h"
#include "graphics/js_canvas.h"
#include "js/js_obj.h"
#include "js/js_utils.h"
#include "util/debug.h"

canvas_t *canvas_obj_get_canvas(obj_t *o)
{
    int canvas_id;
   
    if (obj_get_property_int(&canvas_id, o, &Scanvas_id))
        return NULL;

    return canvas_get_by_id(canvas_id);
}

int do_canvas_pixel_draw(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    u16 x, y, color;
    canvas_t *c;

    if (argc != 4)
        return js_invalid_args(ret);

    if (!(c = canvas_obj_get_canvas(this)))
    {
        tp_err(("'this' is not a valid canvas object\n"));
        return js_invalid_args(ret);
    }

    x = (u16)obj_get_int(argv[1]);
    y = (u16)obj_get_int(argv[2]);
    color = (u16)obj_get_int(argv[3]);

    canvas_pixel_set(c, x, y, color);

    *ret = UNDEF;
    return 0;
}

int do_canvas_flip(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    canvas_t *c;

    if (argc != 1)
        return js_invalid_args(ret);

    if (!(c = canvas_obj_get_canvas(this)))
    {
        tp_err(("'this' is not a valid canvas object\n"));
        return js_invalid_args(ret);
    }

    canvas_flip(c);

    *ret = UNDEF;
    return 0;
}

int do_canvas_fill(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    u16 color;
    canvas_t *c;

    if (argc != 2)
        return js_invalid_args(ret);

    if (!(c = canvas_obj_get_canvas(this)))
    {
        tp_err(("'this' is not a valid canvas object\n"));
        return js_invalid_args(ret);
    }

    color = (u16)obj_get_int(argv[1]);

    canvas_fill(c, color);

    *ret = UNDEF;
    return 0;
}

int canvas_obj_constructor(canvas_t *canvas, obj_t **ret, obj_t *this,
    int argc, obj_t *argv[])
{
    *ret = object_new();
    obj_set_property_int(*ret, Scanvas_id, canvas->id);
    obj_inherit(*ret, argv[0]);
    return 0;
}
