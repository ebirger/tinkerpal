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

/* Uses Bresenham Algorithm.
 * Adapted from http://en.wikipedia.org/wiki/Midpoint_circle_algorithm 
 */

void circle_draw(canvas_t *c, int x0, int y0, int radius, u16 color)
{
    int error = 1 - radius;
    int errorY = 1;
    int errorX = -2 * radius;
    int x = radius, y = 0;

    canvas_pixel_set(c, x0, y0 + radius, color);
    canvas_pixel_set(c, x0, y0 - radius, color);
    canvas_pixel_set(c, x0 + radius, y0, color);
    canvas_pixel_set(c, x0 - radius, y0, color);

    while (y < x)
    {
        if (error > 0)
        { 
            /* >= 0 produces a slimmer circle. 
             * =0 produces the circle at radius 11
             */
            x--;
            errorX += 2;
            error += errorX;
        }
        y++;
        errorY += 2;
        error += errorY;    
        canvas_pixel_set(c, x0 + x, y0 + y, color);
        canvas_pixel_set(c, x0 - x, y0 + y, color);
        canvas_pixel_set(c, x0 + x, y0 - y, color);
        canvas_pixel_set(c, x0 - x, y0 - y, color);
        canvas_pixel_set(c, x0 + y, y0 + x, color);
        canvas_pixel_set(c, x0 - y, y0 + x, color);
        canvas_pixel_set(c, x0 + y, y0 - x, color);
        canvas_pixel_set(c, x0 - y, y0 - x, color);
    }
}
