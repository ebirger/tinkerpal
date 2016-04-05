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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "doc/print.h"
#include "util/tp_misc.h"
#include "version_data.h"

#define BOARD_FILE "doc/boards.h"

static void print_header(void)
{
    P("---");
    P("title: Boards Guide | Tinkerpal " TINKERPAL_VERSION);
    P("markdown2extras: tables, wiki-tables, code-friendly");
    P("---");
    P("#Boards");
    P("The following details the default settings for various devices "
       "connected to boards");
    P("");
}

struct board {
    const char *desc;
    const char *chipset;
    struct res {
        enum res_type {
            RES_NONE = 0,
            RES_LEDS = 1,
            RES_CONSOLE = 2,
            RES_SSD1306 = 3,
            RES_MMC = 4,
            RES_ENC28J60 = 5,
            RES_ESP8266 = 6,
            RES_PCD8544 = 7,
            RES_ST7735 = 8,
            RES_SSD1329 = 9,
            RES_DOGS102X6 = 10,
            RES_SDL_SCREEN = 11,
            RES_LAST,
        } type;
        union {
            struct {
                const char *led_gpio;
            };
            struct {
                const char *console_name;
            };
            struct {
                const char *ssd1306_i2c_port;
                const char *ssd1306_i2c_addr;
            };
            struct {
                const char *mmc_spi_port;
                const char *mmc_mosi;
                const char *mmc_cs;
            };
            struct {
                const char *enc28j60_spi_port;
                const char *enc28j60_cs;
                const char *enc28j60_intr;
            };
            struct {
                const char *esp8266_serial_port;
            };
            struct {
                const char *spi_screen_rst;
                const char *spi_screen_cs;
                const char *spi_screen_cd;
                const char *spi_screen_spi_port;
                const char *spi_screen_spi_backlight;
            };
            struct {
                const char *sdl_screen_width;
                const char *sdl_screen_height;
            };
        };
    } *res;
} boards[] = {
#define GPIO_RES(res) #res
#define UART_RES(res) #res
#define I2C_RES(res) #res
#define SPI_RES(res) #res

#define BOARD_START(_desc, _chipset) { \
    .desc = _desc, \
    .chipset = #_chipset, \
    .res = (struct res []){
#define BOARD_END(...) {} } },

#define DEFAULT_CONSOLE(uart) { \
    .type = RES_CONSOLE, \
    .console_name = uart, \
},
#define LED(gpio) { \
    .type = RES_LEDS, \
    .led_gpio = gpio \
},
#define SSD1306_PARAMS(_i2c_port, _i2c_addr) { \
    .type = RES_SSD1306, \
    .ssd1306_i2c_port = _i2c_port, \
    .ssd1306_i2c_addr = #_i2c_addr, \
},
#define MMC_PARAMS(_spi_port, _mosi, _cs) { \
    .type = RES_MMC, \
    .mmc_spi_port = _spi_port, \
    .mmc_mosi = _mosi, \
    .mmc_cs = _cs, \
},
#define ENC28J60_PARAMS(_spi_port, _cs, _intr) { \
    .type = RES_ENC28J60, \
    .enc28j60_spi_port = _spi_port, \
    .enc28j60_cs = _cs, \
    .enc28j60_intr = _intr, \
},
#define ESP8266_PARAMS(_serial_port) { \
    .type = RES_ESP8266, \
    .esp8266_serial_port = _serial_port, \
},
#define PCD8544_PARAMS(_rst, _cs, _cd, _spi_port, _backlight) { \
    .type = RES_PCD8544, \
    .spi_screen_rst = _rst, \
    .spi_screen_cs = _cs, \
    .spi_screen_cd = _cd, \
    .spi_screen_spi_port = _spi_port, \
    .spi_screen_spi_backlight = _backlight, \
},
#define ST7735_PARAMS(_rst, _cs, _cd, _spi_port, _backlight) { \
    .type = RES_ST7735, \
    .spi_screen_rst = _rst, \
    .spi_screen_cs = _cs, \
    .spi_screen_cd = _cd, \
    .spi_screen_spi_port = _spi_port, \
    .spi_screen_spi_backlight = _backlight, \
},
#define DOGS102X6_PARAMS(_rst, _cs, _cd, _spi_port, _backlight) { \
    .type = RES_DOGS102X6, \
    .spi_screen_rst = _rst, \
    .spi_screen_cs = _cs, \
    .spi_screen_cd = _cd, \
    .spi_screen_spi_port = _spi_port, \
    .spi_screen_spi_backlight = _backlight, \
},
#define SSD1329_PARAMS(_rst, _cs, _cd, _spi_port) { \
    .type = RES_SSD1329, \
    .spi_screen_rst = _rst, \
    .spi_screen_cs = _cs, \
    .spi_screen_cd = _cd, \
    .spi_screen_spi_port = _spi_port, \
},
#define SDL_SCREEN_PARAMS(_width, _height) { \
    .type = RES_SDL_SCREEN, \
    .sdl_screen_width = #_width, \
    .sdl_screen_height = #_height, \
},

#include "boards/board_cfg.h"

    {}
};

static struct res_ops {
    const char *name;
    int nfields;
    const char **headers;
    const size_t *offsets;
} res_ops[] = {
#define OFS(f) (offsetof(struct res, f))
#define HDRS(args...) (const char *[]){ args }
#define OFSTS(args...) (const size_t[]){ args }
#define RS(res, nm, hdrs, ofsts) \
    [res] = { \
        .name = nm, \
        .nfields = PP_NARG hdrs, \
        .headers = HDRS hdrs, \
        .offsets = OFSTS ofsts \
    }
    RS(RES_LEDS, "LEDS",
        ("GPIO"),
        (OFS(led_gpio))),
    RS(RES_CONSOLE, "Console",
        ("Console"),
        (OFS(console_name))),
    RS(RES_SSD1306, "SSD1306",
        ("I2C Port",            "I2C Address"),
        (OFS(ssd1306_i2c_port), OFS(ssd1306_i2c_addr))),
    RS(RES_MMC, "MMC",
        ("SPI Port",        "MOSI Pin",    "CS Pin"),
        (OFS(mmc_spi_port), OFS(mmc_mosi), OFS(mmc_cs))),
    RS(RES_ENC28J60, "ENC28J60",
        ("SPI Port",             "CS Pin",         "Int. Pin"),
        (OFS(enc28j60_spi_port), OFS(enc28j60_cs), OFS(enc28j60_intr))),
    RS(RES_ESP8266, "ESP8266 Serial Bridge",
        ("Serial Port"),
        (OFS(esp8266_serial_port))),
    RS(RES_PCD8544, "PCD8544",
        ("SPI Port", "RST Pin", "CS Pin", "CD Pin", "Backlight Pin"),
        (OFS(spi_screen_spi_port), OFS(spi_screen_rst), OFS(spi_screen_cs), OFS(spi_screen_cd), OFS(spi_screen_spi_backlight))),
    RS(RES_ST7735, "ST7735",
        ("SPI Port", "RST Pin", "CS Pin", "CD Pin", "Backlight Pin"),
        (OFS(spi_screen_spi_port), OFS(spi_screen_rst), OFS(spi_screen_cs), OFS(spi_screen_cd), OFS(spi_screen_spi_backlight))),
    RS(RES_DOGS102X6, "DOGS102X6",
        ("SPI Port", "RST Pin", "CS Pin", "CD Pin", "Backlight Pin"),
        (OFS(spi_screen_spi_port), OFS(spi_screen_rst), OFS(spi_screen_cs), OFS(spi_screen_cd), OFS(spi_screen_spi_backlight))),
    RS(RES_SSD1329, "SSD1329",
        ("SPI Port", "RST Pin", "CS Pin", "CD Pin"),
        (OFS(spi_screen_spi_port), OFS(spi_screen_rst), OFS(spi_screen_cs), OFS(spi_screen_cd))),
    RS(RES_SDL_SCREEN, "SDL Screen Emulation",
        ("Width", "Height"),
        (OFS(sdl_screen_width), OFS(sdl_screen_height))),
};

static void res_ops_print_headers(struct res_ops *ops)
{
    __print_table_header(ops->nfields, ops->headers);
}

static void res_ops_print_row(struct res_ops *ops, struct res *r)
{
    const char **ptrs = malloc(ops->nfields * sizeof(char *));
    int i;

    for (i = 0; i < ops->nfields; i++)
        ptrs[i] = *((const char **)((unsigned long)r + ops->offsets[i]));

    __print_table_row(ops->nfields, ptrs);

    free(ptrs);
}

static void print_res_by_type(struct res *list, enum res_type type)
{
    struct res_ops *ops = &res_ops[type];
    struct res *r;

    for (r = list; r->type && r->type != type; r++);
    if (!r->type)
        return;

    print_subsection("%s", ops->name);

    res_ops_print_headers(ops);

    for (r = list; r->type; r++)
    {
        if (r->type != type)
            continue;

        res_ops_print_row(ops, r);
    }
}

static void print_boards(void)
{
    struct board *b;

    for (b = boards; b->desc; b++)
    {
        enum res_type t;

        print_section("%s", b->desc);
        if (strcmp(b->chipset, "NA"))
        {
            P("Chipset [%s](/page/chipset_guide.html#%s)\n", b->chipset,
                b->chipset);
        }

        for (t = RES_NONE + 1; t != RES_LAST; t++)
            print_res_by_type(b->res, t);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    if (!(fp = fopen(argv[1], "w")))
        exit(1);

    print_header();

    print_boards();

    fclose(fp);
    return 0;
}
