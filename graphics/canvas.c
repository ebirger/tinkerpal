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
#include "util/debug.h"
#include "graphics/canvas.h"

static void swap(u16 *a, u16 *b)
{
    u16 tmp = *a;

    *a = *b;
    *b = tmp;
}

static void cap(u16 *a, u16 lo, u16 hi)
{
    if (*a < lo)
        *a = lo;
    else if (*a > hi)
        *a = hi;
}

void canvas_hline(canvas_t *c, u16 x0, u16 x1, u16 y, u16 val)
{
    u16 w = c->width - 1;
    int i;

    if (y > c->height - 1)
        return;

    if (x0 > x1)
        swap(&x0, &x1);

    cap(&x0, 0, w - 1);
    cap(&x1, 0, w - 1);

    if (c->ops->hline)
    {
        c->ops->hline(c, x0, x1, y, val);
        return;
    }

    for (i = x0; i < x1; i++)
    {
        /* Skipping sanity checks */
        c->ops->pixel_set(c, i, y, val);
    }
}

void canvas_vline(canvas_t *c, u16 x, u16 y0, u16 y1, u16 val)
{
    u16 h = c->height - 1;
    int j;

    if (x > c->width - 1)
        return;

    if (y0 > y1)
        swap(&y0, &y1);

    cap(&y0, 0, h - 1);
    cap(&y1, 0, h - 1);

    if (c->ops->vline)
    {
        c->ops->vline(c, x, y0, y1, val);
        return;
    }

    for (j = y0; j < y1; j++)
    {
        /* Skipping sanity checks */
        c->ops->pixel_set(c, x, j, val);
    }
}

void canvas_fill(canvas_t *c, u16 val)
{
    int i, j;

    if (c->ops->fill)
    {
        c->ops->fill(c, val);
        return;
    }

    for (i = 0; i < c->width; i++)
    {
        for (j = 0; j < c->height; j++)
        {
            /* Skipping sanity checks */
            c->ops->pixel_set(c, i, j, val);
        }
    }
}
