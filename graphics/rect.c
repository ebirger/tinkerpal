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
#include "graphics/graphics.h"

void rect_draw(canvas_t *c, int x, int y, int w, int h, u16 color)
{
    canvas_hline(c, x, x + w, y, color);
    canvas_vline(c, x + w, y, y + h, color);
    canvas_hline(c, x + w, x, y + h, color);
    canvas_vline(c, x, y + h, y, color);
}

void round_rect_draw(canvas_t *c, int x, int y, int w, int h, int r, int type,
    u16 color)
{
    /* Corners */
    switch (type)
    {
    case ROUND_RECT_TYPE_REGULAR:
        _circle_draw(c, x + r, y + r, r, CIRC_270_315 | CIRC_315_0, color);
        _circle_draw(c, x + w - r, y + r, r, CIRC_0_45 | CIRC_45_90, color);
        _circle_draw(c, x + r, y + h - r, r, CIRC_180_225 | CIRC_225_270,
            color);
        _circle_draw(c, x + w - r, y + h - r, r, CIRC_90_135 | CIRC_135_180,
            color);
        break;
    case ROUND_RECT_TYPE_CORNERS_IN:
        _circle_draw(c, x, y, r, CIRC_90_135 | CIRC_135_180, color);
        _circle_draw(c, x + w, y, r, CIRC_180_225 | CIRC_225_270, color);
        _circle_draw(c, x, y + h, r, CIRC_0_45 | CIRC_45_90, color);
        _circle_draw(c, x + w, y + h, r, CIRC_270_315 | CIRC_315_0, color);
        break;
    }
    /* Lines */
    canvas_hline(c, x + r, x + w - r, y, color);
    canvas_vline(c, x + w, y + r, y + h - r, color);
    canvas_hline(c, x + w - r, x + r, y + h, color);
    canvas_vline(c, x, y + h - r, y + r, color);
}

void rect_fill(canvas_t *c, int x, int y, int w, int h, u16 color)
{
    int j;

    for (j = 0; j < h; j++)
        canvas_hline(c, x, x + w, y + j, color);
}
