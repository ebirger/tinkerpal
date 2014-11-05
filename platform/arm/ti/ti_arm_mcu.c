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
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"
#include "platform/platform.h"
#ifdef CONFIG_GPIO
#include "drivers/gpio/gpio_platform.h"
#endif
#include "platform/arm/cortex-m.h"
#include "platform/arm/ti/ti_arm_mcu.h"
#ifdef CONFIG_STELLARIS_ETH
#include "platform/arm/ti/stellaris_eth.h"
#endif
#ifdef CONFIG_TIVA_C_ETH
#include "platform/arm/ti/tiva_c_emac.h"
#endif

#define SYSTEM_CLOCK() platform.get_system_clock()

void ti_arm_mcu_systick_init(void)
{
    MAP_SysTickPeriodSet(SYSTEM_CLOCK() / 1000);
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();
    MAP_IntMasterEnable();
}

volatile unsigned int ctrl_dummy;

void ti_arm_mcu_systick_isr(void)
{
    extern void cortex_m_systick_isr(void);
    cortex_m_systick_isr();
    ctrl_dummy = HWREG(NVIC_ST_CTRL); /* Clear the 'count' bit */
}

void ti_arm_mcu_get_time_from_boot(uint32_t *sec, uint32_t *usec)
{
    do
    {
	uint32_t current, tmp_usec;

	cortex_m_get_time_from_boot(sec, &tmp_usec);
	current = MAP_SysTickValueGet();
	cortex_m_get_time_from_boot(sec, usec);

	if ((HWREG(NVIC_ST_CTRL) & NVIC_ST_CTRL_COUNT) || tmp_usec != *usec)
	{
	    /* Wrap around. Retry */
	    continue;
	}

	/* adjust usec to sub ms resolution */
	*usec += 1000000 - (current / (SYSTEM_CLOCK() / 1000000));
	if (*usec >= 1000000)
	{
	    /* Assuming only one wrap around... */
	    *usec -= 1000000;
	    (*sec)++;
	}
    } while (0);
}

unsigned long ti_arm_mcu_get_system_clock(void)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    return MAP_SysCtlClockGet();
#elif defined(CONFIG_CC3200)
    return 800000;
#endif
}

void ti_arm_mcu_msleep(double ms)
{
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    MAP_SysCtlDelay(SYSTEM_CLOCK() * ms / 3040);
#elif defined(CONFIG_CC3200)
    tp_crit(("%s not implemented yet\n", __FUNCTION__));
#endif
}

void ti_arm_mcu_uart_isr(int u)
{
    unsigned long istat;
    unsigned long base = ti_arm_mcu_uarts[u].base;

    /* Get and clear the current interrupts */
    istat = MAP_UARTIntStatus(base, 1);
    MAP_UARTIntClear(base, istat);

    /* Only handle recieved characters */
    if (!(istat & (UART_INT_RX | UART_INT_RT)))
        return;

    /* Get all the available characters from the UART */
    while (MAP_UARTCharsAvail(base))
    {
        char c;
        long l;

        /* Read a character */
        l = MAP_UARTCharGetNonBlocking(base);
        c = (unsigned char)(l & 0xFF);

        if (buffered_serial_push(u, c))
        {
            /* No space left */
            break;
        }
    }
}

void ti_arm_mcu_serial_irq_enable(int u, int enable)
{
    if (enable)
        MAP_IntEnable(ti_arm_mcu_uarts[u].irq);
    else
        MAP_IntDisable(ti_arm_mcu_uarts[u].irq);
}

int ti_arm_mcu_serial_write(int u, char *buf, int size)
{
    for (; size--; buf++)
        MAP_UARTCharPut(ti_arm_mcu_uarts[u].base, *buf);

    return 0;
}

static inline void ti_arm_mcu_pin_mode_uart(int pin, int uart_af)
{
    ti_arm_mcu_periph_enable(ti_arm_mcu_gpio_periph(pin));
#if defined(CONFIG_STELLARIS) || defined(CONFIG_TIVA_C)
    if (uart_af)
        MAP_GPIOPinConfigure(uart_af);
    MAP_GPIOPinTypeUART(ti_arm_mcu_gpio_base(pin), GPIO_BIT(pin));
#elif defined(CONFIG_CC3200)
    MAP_PinTypeUART(pin, PIN_MODE_3);
#endif
}

int ti_arm_mcu_uart_enable(int u, int enabled)
{
    const ti_arm_mcu_uart_t *uart = &ti_arm_mcu_uarts[u];

    if (!enabled)
    {
        MAP_UARTDisable(uart->base);
        MAP_IntDisable(uart->irq);
        MAP_UARTIntDisable(uart->base, 0xFFFFFFFF);
        ti_arm_mcu_periph_disable(uart->periph);
        return 0;
    }

    ti_arm_mcu_pin_mode_uart(uart->rxpin, uart->rx_af);
    ti_arm_mcu_pin_mode_uart(uart->txpin, uart->tx_af);

    ti_arm_mcu_periph_enable(uart->periph);
    MAP_UARTConfigSetExpClk(uart->base, SYSTEM_CLOCK(), 115200, 
        UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8);

    /* Set the UART to interrupt whenever the TX FIFO is almost empty or
     * when any character is received
     */
    MAP_UARTFIFOLevelSet(uart->base, UART_FIFO_TX1_8, UART_FIFO_RX1_8);

    /* We are configured for buffered output so enable the master interrupt
     * for this UART and the receive interrupts.
     */
    MAP_UARTIntDisable(uart->base, 0xFFFFFFFF);
    MAP_UARTIntEnable(uart->base, UART_INT_RX | UART_INT_RT);
    MAP_IntEnable(uart->irq);
    MAP_UARTEnable(uart->base);
    return 0;
}

int ti_arm_mcu_uart_set_params(int u, const serial_params_t *params)
{
    const ti_arm_mcu_uart_t *uart = &ti_arm_mcu_uarts[u];

    platform_msleep(1); /* XXX: letting things settle */
    MAP_UARTDisable(uart->base);
    MAP_UARTConfigSetExpClk(uart->base, SYSTEM_CLOCK(), params->baud_rate, 
        UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | UART_CONFIG_WLEN_8);
    MAP_UARTEnable(uart->base);
    return 0;
}

int ti_arm_mcu_select(int ms)
{
    uint64_t expire = platform_get_ticks_from_boot() + ms;
    int event = 0;

    while ((!ms || platform_get_ticks_from_boot() < expire) && !event)
    {
#ifdef CONFIG_GPIO
        event |= gpio_events_process();
#endif
#ifdef CONFIG_STELLARIS_ETH
        event |= stellaris_eth_event_process();
#endif
#ifdef CONFIG_TIVA_C_ETH
        event |= tiva_c_emac_event_process();
#endif
#ifdef CONFIG_USB_DEVICE
        event |= ti_arm_mcu_usbd_event_process();
#endif
        event |= buffered_serial_events_process();

        ti_arm_mcu_sleep();
    }

    return event;
}
