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
#include "drivers/lcd/ssd1306.h"
#include "drivers/gpio/gpio.h"
#include "drivers/spi/spi.h"
#include "graphics/colors.h"

typedef struct {
    ssd1306_params_t params;
    canvas_t canvas;
} ssd1306_t;

static ssd1306_t g_ssd1306_screen;

static void chip_init(ssd1306_t *screen)
{
    tp_out(("SSD1306 chip init!\n"));
}

static void ssd1306_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
}

static const canvas_ops_t ssd1306_ops = {
    .pixel_set = ssd1306_pixel_set,
};

canvas_t *ssd1306_new(const ssd1306_params_t *params)
{
    ssd1306_t *screen = &g_ssd1306_screen;

    screen->params = *params;

    chip_init(screen);

    screen->canvas.width = 128;
    screen->canvas.height = 64;
    screen->canvas.ops = &ssd1306_ops;

    canvas_register(&screen->canvas);
    return &screen->canvas;
}
