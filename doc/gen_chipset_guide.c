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

#define PLATFORM_CHIPSET_H "doc/chips.h"

static void print_header(void)
{
    P("---");
    P("title: Chipset Guide | Tinkerpal " TINKERPAL_VERSION);
    P("markdown2extras: tables, wiki-tables, code-friendly");
    P("---");
    P("#Chipset Guide");
    P("Chipsets preperties detailed below");
    P("");
}

struct chip {
    const char *name;
    struct res {
        enum res_type {
            RES_NONE = 0,
            RES_ARM_MEM_AREA = 1,
            RES_UART = 2,
            RES_I2C = 3,
            RES_SSI = 4,
            RES_SPI = 5,
            RES_USCI = 6,
            RES_USBD = 7,
            RES_LAST,
        } type;
        union {
            struct {
                const char *uart_name;
                const char *uart_rx;
                const char *uart_tx;
            };
            struct {
                const char *i2c_name;
                const char *i2c_scl;
                const char *i2c_sda;
            };
            struct {
                const char *ssi_name;
                const char *ssi_clk;
                const char *ssi_fss;
                const char *ssi_rx;
                const char *ssi_tx;
            };
            struct {
                const char *spi_name;
                const char *spi_clk;
                const char *spi_miso;
                const char *spi_mosi;
            };
            struct {
                const char *usci_name;
                const char *usci_rx;
                const char *usci_tx;
                const char *usci_clk;
            };
            struct {
                const char *usbd_dp;
                const char *usbd_dm;
            };
            struct {
                const char *arm_mem_area_name;
                const char *arm_mem_area_perms;
                const char *arm_mem_area_addr;
                const char *arm_mem_area_size;
            };
        };
    } *res;
} chips[] = {
#define CHIPSET_START(chip) { .name = #chip, .res = (struct res []){
#define CHIPSET_END() {} } },
#define TI_UART_DEF(num, rx, tx) { \
    .type = RES_UART, \
    .uart_name = "UART" #num, \
    .uart_rx = #rx, \
    .uart_tx = #tx \
},
#define STM32_USART_DEF(num, type_name, rx, tx, afsig, apb) { \
    .type = RES_UART, \
    .uart_name = #type_name #num, \
    .uart_rx = #rx, \
    .uart_tx = #tx \
},
#define TI_I2C_DEF(num, scl, sda) { \
    .type = RES_I2C, \
    .i2c_name = "I2C" #num, \
    .i2c_scl = #scl, \
    .i2c_sda = #sda \
},
#define STM32_I2C_DEF(num, apb, sclpin, sdapin) { \
    .type = RES_I2C, \
    .i2c_name = "I2C" #num, \
    .i2c_scl = #sclpin, \
    .i2c_sda = #sdapin \
},
#define TI_SSI_DEF(num, clkpin, fsspin, rxpin, txpin) { \
    .type = RES_SSI, \
    .ssi_name = "SSI" #num, \
    .ssi_clk = #clkpin, \
    .ssi_fss = #fsspin, \
    .ssi_rx = #rxpin, \
    .ssi_tx = #txpin \
},
#define STM32_SPI_DEF(num, apb, clkpin, misopin, mosipin, afsig) { \
    .type = RES_SPI, \
    .spi_name = "SPI" #num, \
    .spi_clk = #clkpin, \
    .spi_miso = #misopin, \
    .spi_mosi = #mosipin, \
},
#define MSP430_USCI_DEF(id, rx, tx, clk) { \
    .type = RES_USCI, \
    .usci_name = "USCI" #id, \
    .usci_rx = #rx, \
    .usci_tx = #tx, \
    .usci_clk = #clk, \
},
#define TI_USBD_DEF(dp, dm) { \
    .type = RES_USBD, \
    .usbd_dp = #dp, \
    .usbd_dm = #dm, \
},
#define ARM_MEMORY_AREA(name, perms, addr, size) { \
    .type = RES_ARM_MEM_AREA, \
    .arm_mem_area_name = #name, \
    .arm_mem_area_perms = #perms, \
    .arm_mem_area_addr = #addr, \
    .arm_mem_area_size = #size, \
},

#include "platform/chipset.h"

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
    RS(RES_UART, "UART",
        ("UART",         "RX Pin",     "TX Pin"),
        (OFS(uart_name), OFS(uart_rx), OFS(uart_tx))),
    RS(RES_I2C, "I2C",
        ("I2C",          "SCL Pin",    "SDA Pin"),
        (OFS(i2c_name),  OFS(i2c_scl), OFS(i2c_sda))),
    RS(RES_SSI, "SSI",
        ("SSI",          "FSS Pin",    "CLK Pin",     "RX Pin",    "TX Pin"),
        (OFS(ssi_name),  OFS(ssi_fss), OFS(ssi_clk),  OFS(ssi_rx), OFS(ssi_tx))),
    RS(RES_USCI, "USCI",
        ("USCI",         "RX Pin",     "TX Pin",      "CLK Pin"),
        (OFS(usci_name), OFS(usci_rx), OFS(usci_tx),  OFS(usci_clk))),
    RS(RES_SPI, "SPI",
        ("SPI",          "CLK Pin",    "MISO Pin",    "MOSI Pin"),
        (OFS(spi_name),  OFS(spi_clk), OFS(spi_miso), OFS(spi_mosi))),
    RS(RES_USBD, "USBD",
        ("DP Pin",       "DM Pin"),
        (OFS(usbd_dp),   OFS(usbd_dm))),
    RS(RES_ARM_MEM_AREA, "Memory Areas",
        ("Name",                 "Permissions",           "Address",              "Size"),
        (OFS(arm_mem_area_name), OFS(arm_mem_area_perms), OFS(arm_mem_area_addr), OFS(arm_mem_area_size)))
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

static void print_pin_mappings(void)
{
    struct chip *c;

    for (c = chips; c->name; c++)
    {
        enum res_type t;

        print_section("%s", c->name);

        for (t = RES_NONE + 1; t != RES_LAST; t++)
            print_res_by_type(c->res, t);
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

    print_pin_mappings();

    fclose(fp);
    return 0;
}
