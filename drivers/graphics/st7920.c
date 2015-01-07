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
#define LCD_HEIGHT 64
#define LCD_BANKS (LCD_WIDTH / 16)

#define ST7920_CLEAR 0x01
#define ST7920_DISP_ON 0x0c
#define ST7920_SET_8_BIT 0x30
#define ST7920_EXT_MODE 0x04
#define ST7920_GR_ON 0x02
#define ST7920_SET_GDRAM_ADDR 0x80
#define ST7920_ENTRY_MODE 0x06

typedef struct {
    canvas_t canvas;
    st7920_params_t params;
    u16 shadow[LCD_BANKS * LCD_HEIGHT];
} st7920_t;

static st7920_t g_st7920_screen;

#define PIN_MODE_SET(pin, mode) do { \
    if (gpio_set_pin_mode(pin, mode)) \
        tp_crit(("st7920: unable to set mode %d for pin %x\n", mode, pin)); \
} while(0)

static void st7920_data_out(st7920_t *screen, u8 data)
{
    gpio_digital_write(screen->params.d[0], data & 0x01);
    gpio_digital_write(screen->params.d[1], data & 0x02);
    gpio_digital_write(screen->params.d[2], data & 0x04);
    gpio_digital_write(screen->params.d[3], data & 0x08);
    gpio_digital_write(screen->params.d[4], data & 0x10);
    gpio_digital_write(screen->params.d[5], data & 0x20);
    gpio_digital_write(screen->params.d[6], data & 0x40);
    gpio_digital_write(screen->params.d[7], data & 0x80);
}

static void st7920_wait(st7920_t *screen)
{
    int busy = 1;

    PIN_MODE_SET(screen->params.d[7], GPIO_PM_INPUT_PULLUP);
    gpio_digital_write(screen->params.rw, 1);
    gpio_digital_write(screen->params.rs, 0);
    while (busy)
    {
        gpio_digital_write(screen->params.en, 1);
        busy &= (gpio_digital_read(screen->params.d[7]) ? 1 : 0);
        gpio_digital_write(screen->params.en, 0);
    }
    PIN_MODE_SET(screen->params.d[7], GPIO_PM_OUTPUT);
}

static void st7920_write(st7920_t *screen, int iscmd, u8 data)
{
    st7920_wait(screen);
    gpio_digital_write(screen->params.rs, iscmd ? 0 : 1);
    gpio_digital_write(screen->params.rw, 0);
    st7920_data_out(screen, data);
    gpio_digital_pulse(screen->params.en, 1, 0.01);
}

static void chip_init(st7920_t *screen)
{
    int i;

    PIN_MODE_SET(screen->params.rs, GPIO_PM_OUTPUT);
    PIN_MODE_SET(screen->params.rw, GPIO_PM_OUTPUT);
    PIN_MODE_SET(screen->params.en, GPIO_PM_OUTPUT);
    PIN_MODE_SET(screen->params.rst, GPIO_PM_OUTPUT);
    PIN_MODE_SET(screen->params.psb, GPIO_PM_OUTPUT);
    for (i = 0; i < 8; i++)
        PIN_MODE_SET(screen->params.d[i], GPIO_PM_OUTPUT);
    
    /* Parallel interface */
    gpio_digital_write(screen->params.psb, 1);

    /* Reset */
    gpio_digital_pulse(screen->params.rst, 0, 10);

    st7920_write(screen, 1, ST7920_SET_8_BIT);
    platform_msleep(0.5);
    st7920_write(screen, 1, ST7920_SET_8_BIT | ST7920_EXT_MODE);
    platform_msleep(0.5);
    st7920_write(screen, 1, ST7920_SET_8_BIT | ST7920_EXT_MODE| ST7920_GR_ON);
    st7920_write(screen, 1, ST7920_DISP_ON);
    st7920_write(screen, 1, ST7920_ENTRY_MODE);
    st7920_write(screen, 1, ST7920_CLEAR);
}

static void st7920_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    st7920_t *screen = container_of(c, st7920_t, canvas);
    u16 bank_bit;
    u8 bank;

    bank = x >> 4; /* Each 16 horizontal dots are set together */
    bank_bit = 0x8000 >> (x & 0x0f);

    /* Set shadow */
    if (val)
        screen->shadow[y * LCD_BANKS + bank] |= bank_bit;
    else
        screen->shadow[y * LCD_BANKS + bank] &= ~bank_bit;
}

static void st7920_set_gdram_row(st7920_t *screen, u8 row)
{
    if (row < 32)
    {
        st7920_write(screen, 1, ST7920_SET_GDRAM_ADDR | row);
        st7920_write(screen, 1, ST7920_SET_GDRAM_ADDR | 0);
    }
    else
    {
        st7920_write(screen, 1, ST7920_SET_GDRAM_ADDR | (row - 32));
        st7920_write(screen, 1, ST7920_SET_GDRAM_ADDR | 0x08);
    }
}

static void st7920_flip(canvas_t *c)
{
    st7920_t *screen = container_of(c, st7920_t, canvas);
    u16 *v = screen->shadow;
    u8 y, b;

    for (y = 0; y < LCD_HEIGHT; y++)
    {
        st7920_set_gdram_row(screen, y);

        for (b = 0; b < LCD_BANKS; b++)
        {
            st7920_write(screen, 0, *v >> 8);
            st7920_write(screen, 0, *v & 0xff);
            v++;
        }
    }
}

static void st7920_fill(canvas_t *c, u16 val)
{
    st7920_t *screen = container_of(c, st7920_t, canvas);

    memset(screen->shadow, val ? 0xff : 0, sizeof(screen->shadow));
}

static const canvas_ops_t st7920_ops = {
    .pixel_set = st7920_pixel_set,
    .fill = st7920_fill,
    .flip = st7920_flip,
};

canvas_t *st7920_new(const st7920_params_t *params)
{
    st7920_t *screen = &g_st7920_screen;

    screen->params = *params;

    chip_init(screen);

    memset(screen->shadow, 0, sizeof(screen->shadow));
    screen->canvas.width = LCD_WIDTH;
    screen->canvas.height = LCD_HEIGHT;
    screen->canvas.ops = &st7920_ops;
    return &screen->canvas;
}
