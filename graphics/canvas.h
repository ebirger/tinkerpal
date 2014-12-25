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
#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "util/tp_types.h"

typedef struct canvas_t canvas_t;
    
typedef struct {
    void (*pixel_set)(canvas_t *c, u16 x, u16 y, u16 val);
    void (*hline)(canvas_t *c, u16 x0, u16 x1, u16 y, u16 val);
    void (*vline)(canvas_t *c, u16 x0, u16 y0, u16 y1, u16 val);
    void (*fill)(canvas_t *c, u16 val);
    void (*flip)(canvas_t *c);
    void (*free)(canvas_t *c);
} canvas_ops_t;

struct canvas_t {
    const canvas_ops_t *ops;
    u16 width;
    u16 height;
};

static inline void canvas_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    if ((s16)x < 0 || x >= c->width || (s16)y < 0 || y >= c->height)
        return;

    c->ops->pixel_set(c, x, y, val);
}

static inline void canvas_flip(canvas_t *c)
{
    if (c->ops->flip)
        c->ops->flip(c);
}

void canvas_hline(canvas_t *c, u16 x0, u16 x1, u16 y, u16 val);
void canvas_vline(canvas_t *c, u16 x, u16 y0, u16 y1, u16 val);
void canvas_fill(canvas_t *c, u16 val);

static inline void canvas_free(canvas_t *c)
{
    if (c->ops->free)
        c->ops->free(c);
}

#endif
