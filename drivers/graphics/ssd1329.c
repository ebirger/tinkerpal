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
#include "drivers/graphics/ssd1329.h"
#include "drivers/gpio/gpio.h"
#include "drivers/spi/spi.h"
#include "graphics/colors.h"

#define WIDTH 128
#define HEIGHT 96

typedef struct {
    ssd1329_params_t params;
    canvas_t canvas;
    u8 shadow[(WIDTH / 2) * HEIGHT];
} ssd1329_t;

static const u8 ssd1329_init_seq[] = {
    3, 0xfd, 0x12, 0xe3, /* Unlock commands */
    2, 0xae, 0xe3, /* Display off */
    3, 0x94, 0, 0xe3, /* Icon off */
    3, 0xa8, 95, 0xe3, /* Multiplex ratio */
    3, 0x81, 0xb7, 0xe3, /* Contrast */
    3, 0x82, 0x3f, 0xe3, /* Pre-charge current */
    3, 0xa0, 0x52, 0xe3, /* Display Re-map */
    3, 0xa1, 0, 0xe3, /* Display Start Line */
    3, 0xa2, 0x00, 0xe3, /* Display Offset */
    2, 0xa4, 0xe3, /* Display Mode Normal */
    3, 0xb1, 0x11, 0xe3, /* Phase Length */
    3, 0xb2, 0x23, 0xe3, /* Frame frequency */
    3, 0xb3, 0xe2, 0xe3, /* Front Clock Divider */
    /* Set gray scale table.  App note uses default command:
     * 2, 0Xb7, 0xe3
     * This gray scale attempts some gamma correction to reduce the
     * the brightness of the low levels.
     */
    17, 0xb8, 1, 2, 3, 4, 5, 6, 8, 10, 12, 14, 16, 19, 22, 26, 30, 0xe3,
    3, 0xbb, 0x01, 0xe3, /* Second pre-charge period. App note value - 0x04. */
    3, 0xbc, 0x3f, 0xe3, /* Pre-charge voltage */
    2, 0xaf, 0xe3, /* Display ON */
    0
};

static ssd1329_t g_ssd1329_screen;

static void ssd1329_write(ssd1329_t *screen, int is_cmd, const u8 *data,
    int len)
{
    gpio_digital_write(screen->params.cs, 0);
    gpio_digital_write(screen->params.cd, !is_cmd);

    spi_send_mult(screen->params.spi_port, (u8 *)data, len);

    gpio_digital_write(screen->params.cs, 1);
}

static void chip_init(ssd1329_t *screen)
{
    const u8 *cmd;

    gpio_set_pin_mode(screen->params.cs, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.cd, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.pwr, GPIO_PM_OUTPUT);

    /* Activate Screen */
    gpio_digital_write(screen->params.pwr, 1);

    /* Init SPI */
    gpio_digital_write(screen->params.cs, 0);

    spi_init(screen->params.spi_port);

    /* Send init sequence */
    for (cmd = ssd1329_init_seq; *cmd; cmd += *cmd + 1)
        ssd1329_write(screen, 1, cmd + 1, *cmd);
}

static void ssd1329_set_address(ssd1329_t *screen, u8 min_x, u8 max_x,
    u8 min_y, u8 max_y)
{
    ssd1329_write(screen, 1, (u8 []){ 0x15, min_y, max_y }, 3);
    ssd1329_write(screen, 1, (u8 []){ 0x75, min_x, max_x }, 3);
}

static void ssd1329_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    ssd1329_t *screen = container_of(c, ssd1329_t, canvas);
    u8 cell;

    val &= 0xf;

    /* Set shadow */
    cell = screen->shadow[y * WIDTH + x / 2];
    if (x & 1)
    {
        cell &= ~0xf;
        cell |= val;
    }
    else
    {
        cell &= ~0xf0;
        cell |= val << 4;
    }
    screen->shadow[y * WIDTH + x / 2] = cell;

    ssd1329_set_address(screen, y, y, x / 2, x / 2);

    /* Draw on screen */
    ssd1329_write(screen, 0, &cell, 1);
}

static const canvas_ops_t ssd1329_ops = {
    .pixel_set = ssd1329_pixel_set,
};

canvas_t *ssd1329_new(const ssd1329_params_t *params)
{
    ssd1329_t *screen = &g_ssd1329_screen;

    screen->params = *params;

    chip_init(screen);

    screen->canvas.width = WIDTH;
    screen->canvas.height = HEIGHT;
    screen->canvas.ops = &ssd1329_ops;

    canvas_register(&screen->canvas);
    return &screen->canvas;
}
