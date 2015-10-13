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
#include "platform/platform.h"
#include "platform/ticks.h"
#include "util/tp_misc.h"
#include "ets_sys.h"
#include "mem.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "ip_addr.h"
#include "spi_flash.h"
#include "espconn.h"
#include "user_interface.h"
#include "uart_register.h"

/* Missing function prototype definitions */
extern void uart_div_modify(int no, unsigned int freq);
extern STATUS uart_tx_one_char(uint8 TxChar);
extern void *pvPortZalloc(int);
extern void vPortFree(void *ptr);
extern void ets_isr_attach(int intr, void *handler, void *arg);
extern void ets_isr_mask(unsigned intr);
extern void ets_isr_unmask(unsigned intr);

/* ESP8266 platform code */
static os_event_t task_queue[1];

static void esp8266_meminfo(void)
{
    uint32_t data;
    uint8_t *bytes = (uint8_t *)&data, flash_sz_id;
    static const char *flash_sz_str[] = {
        [0x0] = "512 KB",
        [0x1] = "256 KB",
        [0x2] = "1 MB",
        [0x3] = "2 MB",
        [0x4] = "4 MB",
    };

    tp_out("Heap: Available %d\n", system_get_free_heap_size());

    /* First 4 byte contain a magic byte + flash config */
    if (spi_flash_read(0x0000, &data, sizeof(data)) != SPI_FLASH_RESULT_OK)
    {
        tp_err("esp8266: cannot read flash config\n");
        return;
    }

    flash_sz_id = (bytes[3] & 0xf0) >> 4;
    if (flash_sz_id > ARRAY_SIZE(flash_sz_str) - 1)
    {
        tp_warn("esp8266: Unknown flash size");
        return;
    }

    tp_out("Flash size: %s\n", flash_sz_str[flash_sz_id]);
}

static void esp8266_msleep(double ms)
{
}

static void esp8266_init(void)
{
}

static int esp8266_select(int ms)
{
    return buffered_serial_events_process();
}

void uart0_rx_intr_handler(void *para)
{
    if ((READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST) !=
        UART_RXFIFO_FULL_INT_ST)
    {
        return;
    }

    WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);

    while (READ_PERI_REG(UART_STATUS(UART0)) &
        (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
    {
        uint8 c = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;

        buffered_serial_push(UART0, c);
    }
}

static int esp8266_uart_enable(int u, int enabled)
{
    ETS_UART_INTR_ATTACH(uart0_rx_intr_handler, NULL);
    PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);

    uart_div_modify(0, UART_CLK_FREQ / 115200);

    /* Clear rx and tx fifo */
    SET_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    CLEAR_PERI_REG_MASK(UART_CONF0(UART0), UART_RXFIFO_RST | UART_TXFIFO_RST);
    /* Clear all UART interrupts */
    WRITE_PERI_REG(UART_INT_CLR(UART0), 0xffff);
    /* Enable UART RX_interrupt */
    SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_RXFIFO_FULL_INT_ENA);
    ETS_UART_INTR_ENABLE();
    return 0;
}

static int esp8266_serial_write(int u, char *buf, int size)
{
    while (size--)
        uart_tx_one_char(*buf++);
    return size;
}

static void esp8266_serial_irq_enable(int u, int enable)
{
    if (enable)
        ETS_UART_INTR_ENABLE();
    else
        ETS_UART_INTR_DISABLE();
}

const platform_t platform = {
    .serial = {
        .enable = esp8266_uart_enable,
        .write = esp8266_serial_write,
        .read = buffered_serial_read,
        .irq_enable = esp8266_serial_irq_enable,
    },
    .mem = {
        .malloc = os_zalloc,
        .free = os_free,
        .info = esp8266_meminfo,
    },
    .init = esp8266_init,
    .select = esp8266_select,
    .msleep = esp8266_msleep,
    .get_time_from_boot = gen_get_time_from_boot,
};

static void loop(os_event_t *events)
{
    extern int event_loop_single(int *next_timeout);

    event_loop_single(NULL);
    system_os_post(0, 0, 0);
}

static void system_init_done(void)
{
    extern void app_start(int argc, char *argv[]);
    extern void tp_init(void);

    tp_init();

    tp_out("SDK Version: %s\n", system_get_sdk_version());
    tp_out("CPU Frequency: %d MHz\n", system_get_cpu_freq());
    tp_out("Max TCP Connections: %d\n", espconn_tcp_get_max_con());

    app_start(0, 0);
    system_os_task(loop, 0, task_queue, ARRAY_SIZE(task_queue));
    system_os_post(0, 0, 0);
}

void user_init(void)
{
    system_init_done_cb(system_init_done);
}
