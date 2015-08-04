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
#include "util/cli.h"
#include "platform/platform_consts.h"
#include "boards/board.h"
#include "graphics/graphics.h"
#include "graphics/colors.h"
#include <math.h>

static canvas_t *canvas;

static void graphics_test_process_line(tstr_t *line)
{
    if (!tstr_cmp(line, &S("fill")))
    {
        int i, j;

        for (i = 0; i < canvas->width; i++)
        {
            for (j = 0; j < canvas->height; j++)
                canvas_pixel_set(canvas, i, j, (i * j) & 0xffff);
        }
    }
    if (!tstr_cmp(line, &S("black")))
        canvas_fill(canvas, COLOR_BLACK);
    if (!tstr_cmp(line, &S("white")))
        canvas_fill(canvas, COLOR_WHITE);
    if (!tstr_cmp(line, &S("red")))
        canvas_fill(canvas, COLOR_RED);
    if (!tstr_cmp(line, &S("green")))
        canvas_fill(canvas, COLOR_GREEN);
    if (!tstr_cmp(line, &S("blue")))
        canvas_fill(canvas, COLOR_BLUE);
    if (!tstr_cmp(line, &S("circle")))
    {
        circle_draw(canvas, canvas->width / 2, canvas->height / 2,
            canvas->width / 4, COLOR_WHITE);
    }
    if (!tstr_cmp(line, &S("circle_fill")))
    {
        circle_fill(canvas, canvas->width / 2, canvas->height / 2,
            canvas->width / 4, COLOR_GREEN);
    }
    if (!tstr_cmp(line, &S("text")))
        string_draw(canvas, 10, 10, &S("Hello TinkerPal"), 0xffff);
    if (!tstr_cmp(line, &S("rect")))
    {
        rect_draw(canvas, canvas->width / 4, canvas->height / 4,
            canvas->width / 2, canvas->height / 2, COLOR_WHITE);
    }
    if (!tstr_cmp(line, &S("rrect")))
    {
        round_rect_draw(canvas, canvas->width / 4, canvas->height / 4,
            canvas->width / 2, canvas->height / 2, canvas->width / 10,
            ROUND_RECT_TYPE_REGULAR, COLOR_WHITE);
    }
    if (!tstr_cmp(line, &S("rrect_in")))
    {
        round_rect_draw(canvas, canvas->width / 4, canvas->height / 4,
            canvas->width / 2, canvas->height / 2, canvas->width / 10,
            ROUND_RECT_TYPE_CORNERS_IN, COLOR_WHITE);
    }
    if (!tstr_cmp(line, &S("chart")))
    {
        chart_t *chart;
        int i, j;
        chart_params_t params = {
            .npoints = canvas->width / 3,
            .x = 0,
            .y = 0,
            .w = canvas->width,
            .h = canvas->height,
            .color = COLOR_WHITE,
        };

        chart = chart_new(canvas, &params);
        for (j = 2; j < 50; j++)
        {
            for (i = 0; i < canvas->width / 3; i++)
            {
                s8 p = sin(i * j * 3.14 * 2) * canvas->height / 2;
                chart_add_point(chart, p);
            }
            canvas_flip(canvas);
        }
        chart_free(chart);
    }
    canvas_flip(canvas);
    console_printf("Ok\n");
}

static void graphics_test_quit(void)
{
    if (canvas)
        canvas_free(canvas);
}

static cli_client_t graphics_test_cli_client = {
    .process_line = graphics_test_process_line,
    .quit = graphics_test_quit,
};

static void lcd_init(void)
{
#ifdef CONFIG_ILI93XX
    canvas = ili93xx_new(&board.ili93xx_params);
#elif defined(CONFIG_SDL_SCREEN)
    canvas = sdl_screen_new(&board.sdl_screen_params);
#elif defined(CONFIG_DOGS102X6)
    canvas = dogs102x6_new(&board.dogs102x6_params);
#elif defined(CONFIG_SSD1306)
    canvas = ssd1306_new(&board.ssd1306_params);
#elif defined(CONFIG_SSD1329)
    canvas = ssd1329_new(&board.ssd1329_params);
#elif defined(CONFIG_PCD8544)
    canvas = pcd8544_new(&board.pcd8544_params);
#elif defined(CONFIG_ST7735)
    canvas = st7735_new(&board.st7735_params);
#elif defined(CONFIG_ST7920)
    canvas = st7920_new(&board.st7920_params);
#else
#error No LCD hookup information available
#endif
}

void app_start(int argc, char *argv[])
{
    tp_out("TinkerPal Application - Graphics Test\n");

    lcd_init();

    cli_start(&graphics_test_cli_client);
}
