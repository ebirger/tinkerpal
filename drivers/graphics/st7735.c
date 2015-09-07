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
#include "drivers/graphics/graphics_screens.h"
#include "drivers/gpio/gpio.h"
#include "drivers/spi/spi.h"
#include "graphics/colors.h"

#define LCD_WIDTH 128
#define LCD_HEIGHT 160

typedef struct {
    st7735_params_t params;
    canvas_t canvas;
} st7735_t;

static st7735_t g_st7735_screen;

/* Commands */
#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT 0x11
#define ST7735_NORON 0x13
#define ST7735_INVOFF 0x20
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2a
#define ST7735_RASET 0x2b
#define ST7735_RAMWR 0x2c

#define ST7735_COLMOD 0x3a
#define ST7735_COLMOD_12_BIT 3
#define ST7735_COLMOD_16_BIT 5
#define ST7735_COLMOD_18_BIT 6

/* Memory Data Access Control (MADCTL) */
#define ST7735_MADCTL 0x36
#define ST7735_MADCTL_MH (1<<2) /* Horizontal refresh order */
#define ST7735_MADCTL_RGB (1<<3) /* '0' RGB, '1' BGR */
#define ST7735_MADCTL_ML (1<<4) /* Vertical refresh order */
#define ST7735_MADCTL_MV (1<<5) /* Row/Col exchange */
#define ST7735_MADCTL_MX (1<<6) /* Col address order */
#define ST7735_MADCTL_MY (1<<7) /* Row address order */

/* Inversion Control */
#define ST7735_INVCTR 0xb4
#define ST7735_INVCTR_NLA (1<<2)
#define ST7735_INVCTR_NLB (1<<1)
#define ST7735_INVCTR_NLC (1<<0)

/* Power Control */
#define ST7735_PWCTR1 0xc0
#define ST7735_PWCTR2 0xc1
#define ST7735_PWCTR3 0xc2
#define ST7735_PWCTR4 0xc3
#define ST7735_PWCTR5 0xc4

static void st7735_write(st7735_t *screen, int iscmd, u8 *data, int len)
{
    gpio_digital_write(screen->params.cd, !iscmd);
    spi_send_mult_nss(screen->params.spi_port, screen->params.cs, data, len);
}

static void st7735_cmd(st7735_t *screen, u8 op, int num_params, u8 *params)
{
    st7735_write(screen, 1, &op, 1);
    if (num_params)
        st7735_write(screen, 0, params, num_params);
}

#define DO_CMD(screen, op, args...) \
    st7735_cmd(screen, op, ARRAY_SIZE(((u8 []){ args })), (u8 []){ args })

static void st7735_init_seq(st7735_t *screen)
{
    DO_CMD(screen, ST7735_SWRESET);
    platform_msleep(150);
    DO_CMD(screen, ST7735_SLPOUT);
    platform_msleep(255);
    DO_CMD(screen, ST7735_INVCTR, ST7735_INVCTR_NLA|ST7735_INVCTR_NLB|ST7735_INVCTR_NLC);
    /* Use default values for Power Control unless otherwise specified */
    DO_CMD(screen, ST7735_PWCTR1, 0xa8, 0x08 , 0x84);
    DO_CMD(screen, ST7735_PWCTR2, 0xc0);
    DO_CMD(screen, ST7735_PWCTR3, 0x0a, 0x00);
    DO_CMD(screen, ST7735_PWCTR4, 0x8a, 0x2a /* Use BCLK/2 instead of default BCLK/3 */);
    DO_CMD(screen, ST7735_PWCTR5, 0x8a, 0xee);
    DO_CMD(screen, ST7735_INVOFF);
    DO_CMD(screen, ST7735_MADCTL, ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB);
    DO_CMD(screen, ST7735_COLMOD, ST7735_COLMOD_16_BIT);
    DO_CMD(screen, ST7735_CASET, 0x00, 0x00, 0x00, LCD_WIDTH - 1);
    DO_CMD(screen, ST7735_RASET, 0x00, 0x00, 0x00, LCD_HEIGHT - 1);
    DO_CMD(screen, ST7735_NORON);
    platform_msleep(10);
    DO_CMD(screen, ST7735_DISPON);
    platform_msleep(100);
}

static void chip_init(st7735_t *screen)
{
    gpio_set_pin_mode(screen->params.rst, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.cs, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.cd, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(screen->params.backlight, GPIO_PM_OUTPUT);

    /* Reset */
    gpio_digital_pulse(screen->params.rst, 0, 10);

    gpio_digital_write(screen->params.cd, 0);
    gpio_digital_write(screen->params.cs, 1);
    gpio_digital_write(screen->params.backlight, 1);

    /* Init SPI */
    spi_init(screen->params.spi_port);
    spi_set_max_speed(screen->params.spi_port, 4000000);

    /* Send init sequence */
    st7735_init_seq(screen);
}

void st7735_set_window(st7735_t *screen, u8 x0, u8 y0, u8 x1, u8 y1)
{
    DO_CMD(screen, ST7735_CASET, 0x00, x0, 0x00, x1);
    DO_CMD(screen, ST7735_RASET, 0x00, y0, 0x00, y1);
    DO_CMD(screen, ST7735_RAMWR);
}

static void st7735_fill_window(st7735_t *screen, int n, u16 val)
{
    gpio_digital_write(screen->params.cs, 0);
    gpio_digital_write(screen->params.cd, 1);

    while (n--)
    {
        spi_send(screen->params.spi_port, val >> 8);
        spi_send(screen->params.spi_port, val & 0xff);
    }

    gpio_digital_write(screen->params.cs, 1);
}

static void st7735_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    st7735_t *screen = container_of(c, st7735_t, canvas);

    st7735_set_window(screen, x, y, x + 1, y + 1);
    st7735_fill_window(screen, 1, val);
}

static void st7735_hline(canvas_t *c, u16 x0, u16 x1, u16 y, u16 val)
{
    st7735_t *screen = container_of(c, st7735_t, canvas);

    st7735_set_window(screen, x0, y, x1, y + 1);
    st7735_fill_window(screen, x1 - x0, val);
}

static void st7735_vline(canvas_t *c, u16 x, u16 y0, u16 y1, u16 val)
{
    st7735_t *screen = container_of(c, st7735_t, canvas);

    st7735_set_window(screen, x, y0, x, y1);
    st7735_fill_window(screen, y1 - y0, val);
}

static void st7735_fill(canvas_t *c, u16 val)
{
    st7735_t *screen = container_of(c, st7735_t, canvas);
    int w, h;

    w = screen->canvas.width;
    h = screen->canvas.height;
    st7735_set_window(screen, 0, 0, w - 1, h - 1);
    st7735_fill_window(screen, w * h, val);
}

static const canvas_ops_t st7735_ops = {
    .pixel_set = st7735_pixel_set,
    .hline = st7735_hline,
    .vline = st7735_vline,
    .fill = st7735_fill,
};

canvas_t *st7735_new(const st7735_params_t *params)
{
    st7735_t *screen = &g_st7735_screen;

    screen->params = *params;

    chip_init(screen);

    screen->canvas.width = LCD_WIDTH;
    screen->canvas.height = LCD_HEIGHT;
    screen->canvas.ops = &st7735_ops;
    return &screen->canvas;
}
