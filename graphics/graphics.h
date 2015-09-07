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
#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include "util/tp_types.h"
#include "util/tstr.h"
#include "graphics/canvas.h"

#define CIRC_0_90 (1<<0)
#define CIRC_90_180 (1<<1)
#define CIRC_180_270 (1<<2)
#define CIRC_270_0 (1<<3)
#define CIRC_ALL 0xff
void _circle_draw(canvas_t *c, int x0, int y0, int radius, u8 quad, u16 color);
static inline void circle_draw(canvas_t *c, int x0, int y0, int radius,
    u16 color)
{
    _circle_draw(c, x0, y0, radius, CIRC_ALL, color);
}

void _circle_fill(canvas_t *c, int x0, int y0, int radius, u8 quad, u16 color);
static inline void circle_fill(canvas_t *c, int x0, int y0, int radius,
    u16 color)
{
    _circle_fill(c, x0, y0, radius, CIRC_ALL, color);
}

void string_draw(canvas_t *c, int x, int y, tstr_t *str, u16 color);
void line_draw(canvas_t *c, int x0, int y0, int x1, int y1, u16 color);
void rect_draw(canvas_t *c, int x, int y, int w, int h, u16 color);
void rect_fill(canvas_t *c, int x, int y, int w, int h, u16 color);
#define ROUND_RECT_TYPE_REGULAR 0
#define ROUND_RECT_TYPE_CORNERS_IN 1
void round_rect_draw(canvas_t *c, int x, int y, int w, int h, int r, int type,
    u16 color);
void bitmap_draw(canvas_t *c, int x, int y, int w, int h, const u8 *image);

typedef struct chart_t chart_t;
typedef struct {
    int npoints;
    int x;
    int y;
    int w;
    int h;
    u16 color;
} chart_params_t;

void chart_add_point(chart_t *chart, s8 point);
chart_t *chart_new(canvas_t *c, const chart_params_t *params);
void chart_free(chart_t *chart);

#endif
