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
#include "mem/tmalloc.h"

struct chart_t {
    canvas_t *canvas;
    chart_params_t params;
    int bar_width;
    s8 points[0];
};

static void chart_point(chart_t *chart, int n, s8 old, s8 new)
{
    int x0, x1, j;
    u8 y0, y1, center;
    u16 color;

    center = chart->params.h / 2;
    x0 = chart->params.x + n * chart->bar_width;
    x1 = chart->params.x + (n + 1) * chart->bar_width;
    y0 = chart->params.y + old + center;
    y1 = chart->params.y + new + center;
    color = chart->params.color;

    if (old <= 0 && new <= 0)
    {
        if (old < new)
        {
            for (j = y0; j < y1; j++)
                canvas_hline(chart->canvas, x0, x1, j, 0);
        }
        else
        {
            for (j = y1; j < y0; j++)
                canvas_hline(chart->canvas, x0, x1, j, color);
        }
    }
    else if (old <= 0 && new > 0)
    {
        for (j = y0; j < center; j++)
            canvas_hline(chart->canvas, x0, x1, j, 0);
        for (j = center; j < y1; j++)
            canvas_hline(chart->canvas, x0, x1, j, color);
    }
    else if (old > 0 && new < 0)
    {
        for (j = center; j < y0; j++)
            canvas_hline(chart->canvas, x0, x1, j, 0);
        for (j = y1; j < center; j++)
            canvas_hline(chart->canvas, x0, x1, j, color);
    }
    else if (old > 0 && new > 0)
    {
        if (old > new)
        {
            for (j = y1; j < y0; j++)
                canvas_hline(chart->canvas, x0, x1, j, 0);
        }
        else
        {
            for (j = y0; j < y1; j++)
                canvas_hline(chart->canvas, x0, x1, j, color);
        }
    }
}
    
void chart_add_point(chart_t *chart, s8 point)
{
    s8 old, new;
    int i, npoints;

    npoints = chart->params.npoints;
    for (i = 1; i < npoints; i++)
    {
        old = chart->points[i - 1];
        new = chart->points[i];
        chart->points[i - 1] = new;
        chart_point(chart, i - 1, old, new);
    }
    old = chart->points[npoints - 1];
    chart->points[npoints - 1] = point;
    chart_point(chart, i - 1, old, point);
}

chart_t *chart_new(canvas_t *c, const chart_params_t *params)
{
    int bar_width;
    chart_t *chart;

    if ((bar_width = params->w / params->npoints) < 1)
        return NULL;

    chart = tmalloc(sizeof(*chart) + params->npoints, "chart");
    chart->params = *params;
    chart->bar_width = bar_width;
    chart->canvas = c;
    return chart;
}

void chart_free(chart_t *chart)
{
    tfree(chart);
}
