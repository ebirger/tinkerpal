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
#include "util/tp_misc.h"
#include "util/event.h"
#include "util/debug.h"
#include "drivers/resources.h"
#include "drivers/graphics/graphics_screens.h"
#include "graphics/colors.h"

static canvas_t dummy_canvas;

static void dummy_canvas_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    tp_out("%s: (%d,%d) = %d\n", __func__, x, y, val);
}

static void dummy_canvas_flip(canvas_t *c)
{
    tp_out("%s\n", __func__);
}

static void dummy_canvas_fill(canvas_t *c, u16 val)
{
    tp_out("%s: val %d\n", __func__, val);
}

static const canvas_ops_t dummy_canvas_ops = {
    .pixel_set = dummy_canvas_pixel_set,
    .fill = dummy_canvas_fill,
    .flip = dummy_canvas_flip,
};

canvas_t *dummy_canvas_new(const dummy_canvas_params_t *params)
{
    dummy_canvas.width = 128;
    dummy_canvas.height = 64;
    dummy_canvas.ops = &dummy_canvas_ops;
    return &dummy_canvas;
}
