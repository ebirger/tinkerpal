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
#include "drivers/lcd/pcd8544.h"
#include "drivers/gpio/gpio.h"
#include "drivers/spi/spi.h"
#include "graphics/colors.h"

#define PCD8544_FUNCTION_SET 0x20
#define PCD8544_EXTENDED_INSTRUCTION 0x01

#define PCD8544_DISPLAY_CONTROL 0x08
#define PCD8544_DISPLAY_NORMAL 0x4

#define PCD8544_SET_TEMP 0x04
#define PCD8544_SET_BIAS 0x10
#define PCD8544_SET_VOP 0x80

#define PCD8544_SET_Y_ADDR 0x40
#define PCD8544_SET_X_ADDR 0x80

#define LCD_WIDTH 84
#define LCD_HEIGHT 48

typedef struct {
    pcd8544_params_t params;
    canvas_t canvas;
    u8 shadow[LCD_WIDTH * LCD_HEIGHT / 8];
} pcd8544_t;

static pcd8544_t g_pcd8544_screen;

static u8 pcd8544_init_seq[] = {
    PCD8544_FUNCTION_SET | PCD8544_EXTENDED_INSTRUCTION,
    PCD8544_SET_VOP | 0x3f, /* XXX: support configurable constrast */
    PCD8544_SET_BIAS | 4,
    PCD8544_SET_TEMP | 0x02,
    PCD8544_FUNCTION_SET,
    PCD8544_DISPLAY_CONTROL | PCD8544_DISPLAY_NORMAL,
};

static void pcd8544_write(pcd8544_t *screen, int iscmd, u8 *data, int len)
{
    gpio_digital_write(screen->params.cs, 0);
    gpio_digital_write(screen->params.cd, !iscmd);

    spi_send_mult(screen->params.spi_port, data, len);

    gpio_digital_write(screen->params.cs, 1);
}

static void chip_init(pcd8544_t *screen)
{
    gpio_set_pin_mode(screen->params.rst, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.cs, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.cd, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.backlight, GPIO_PM_OUTPUT);

    /* Reset */
    gpio_digital_pulse(screen->params.rst, 0, 10);

    gpio_digital_write(screen->params.cd, 0);
    gpio_digital_write(screen->params.backlight, 1);

    /* Init SPI */
    spi_init(screen->params.spi_port);
    spi_set_max_speed(screen->params.spi_port, 4000000);

    /* Send init sequence */
    pcd8544_write(screen, 1, pcd8544_init_seq, sizeof(pcd8544_init_seq));
}

static void pcd8544_set_address(pcd8544_t *screen, u8 pa, u8 ca)
{
    u8 cmd[2];

    cmd[0] = PCD8544_SET_Y_ADDR | pa;
    cmd[1] = PCD8544_SET_X_ADDR | ca;
    pcd8544_write(screen, 1, cmd, sizeof(cmd));
}

static void pcd8544_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    pcd8544_t *screen = container_of(c, pcd8544_t, canvas);
    u8 page, line_bit;

    page = y >> 3; /* Each 8 vertical lines are one byte */
    line_bit = 1 << (y & 0x07);

    /* Set shadow */
    if (val)
        screen->shadow[page * LCD_WIDTH + x] |= line_bit;
    else
        screen->shadow[page * LCD_WIDTH + x] &= ~line_bit;

    /* Set address */
    pcd8544_set_address(screen, page, x);

    /* Draw on screen */
    pcd8544_write(screen, 0, screen->shadow + (page * LCD_WIDTH + x), 1);
}

static void pcd8544_flip(canvas_t *c)
{
    pcd8544_t *screen = container_of(c, pcd8544_t, canvas);
    u8 page;

    pcd8544_set_address(screen, 0, 0);
    
    for (page = 0; page < (LCD_HEIGHT >> 3); page++)
        pcd8544_write(screen, 0, screen->shadow + page * LCD_WIDTH, LCD_WIDTH);
}

static void pcd8544_fill(canvas_t *c, u16 val)
{
    pcd8544_t *screen = container_of(c, pcd8544_t, canvas);

    memset(screen->shadow, val ? 0xff : 0, sizeof(screen->shadow));
    pcd8544_flip(c);
}

static const canvas_ops_t pcd8544_ops = {
    .pixel_set = pcd8544_pixel_set,
    .fill = pcd8544_fill,
};

canvas_t *pcd8544_new(const pcd8544_params_t *params)
{
    pcd8544_t *screen = &g_pcd8544_screen;

    screen->params = *params;

    chip_init(screen);

    screen->canvas.width = LCD_WIDTH;
    screen->canvas.height = LCD_HEIGHT;
    screen->canvas.ops = &pcd8544_ops;

    canvas_register(&screen->canvas);
    return &screen->canvas;
}
