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
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/uart.h"
#include "drivers/serial/serial_platform.h"
#ifdef CONFIG_GPIO
#include "drivers/gpio/gpio_platform.h"
#endif
#include "platform/arm/cortex-m.h"
#include "platform/arm/ti/stellaris.h"
#ifdef CONFIG_STELLARIS_ETH
#include "platform/arm/ti/stellaris_eth.h"
#endif

void stellaris_systick_init(void)
{
    MAP_SysTickPeriodSet(MAP_SysCtlClockGet() / 1000);
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();
    MAP_IntMasterEnable();
}

unsigned long stellaris_get_system_clock(void)
{
    return MAP_SysCtlClockGet();
}

void stellaris_msleep(double ms)
{
    MAP_SysCtlDelay(MAP_SysCtlClockGet() * ms / 4000);
}

void stellaris_uart_isr(int u)
{
    unsigned long istat;
    unsigned long base = stellaris_uarts[u].base;

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

void stellaris_serial_irq_enable(int u, int enable)
{
    if (enable)
	MAP_IntEnable(stellaris_uarts[u].irq);
    else
	MAP_IntDisable(stellaris_uarts[u].irq);
}

int stellaris_serial_write(int u, char *buf, int size)
{
    for (; size--; buf++)
        MAP_UARTCharPut(stellaris_uarts[u].base, *buf);

    return 0;
}

static inline void stellaris_pin_mode_uart(int pin)
{
    stellaris_periph_enable(stellaris_gpio_periph(pin));
    if (stellaris_gpio_pins[pin].uart_function != -1)
	MAP_GPIOPinConfigure(stellaris_gpio_pins[pin].uart_function);
    MAP_GPIOPinTypeUART(stellaris_gpio_base(pin), GPIO_BIT(pin));
}

void stellaris_uart_enable(int u, int enabled)
{
    int rxpin = stellaris_uarts[u].rxpin, txpin = stellaris_uarts[u].txpin;

    if (!enabled)
    {
	MAP_UARTDisable(stellaris_uarts[u].base);
	MAP_IntDisable(stellaris_uarts[u].irq);
	MAP_UARTIntDisable(stellaris_uarts[u].base, 0xFFFFFFFF);
	MAP_SysCtlPeripheralDisable(stellaris_uarts[u].periph);
	return;
    }

    stellaris_pin_mode_uart(rxpin);
    stellaris_pin_mode_uart(txpin);

    stellaris_periph_enable(stellaris_uarts[u].periph);
    MAP_UARTConfigSetExpClk(stellaris_uarts[u].base, MAP_SysCtlClockGet(), 
	115200, (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE | 
	UART_CONFIG_WLEN_8));

    /* Set the UART to interrupt whenever the TX FIFO is almost empty or
     * when any character is received
     */
    MAP_UARTFIFOLevelSet(stellaris_uarts[u].base, UART_FIFO_TX1_8, 
	UART_FIFO_RX1_8);

    /* We are configured for buffered output so enable the master interrupt
     * for this UART and the receive interrupts.
     */
    MAP_UARTIntDisable(stellaris_uarts[u].base, 0xFFFFFFFF);
    MAP_UARTIntEnable(stellaris_uarts[u].base, UART_INT_RX | UART_INT_RT);
    MAP_IntEnable(stellaris_uarts[u].irq);
    MAP_UARTEnable(stellaris_uarts[u].base);
}

int stellaris_select(int ms)
{
    int expire = cortex_m_get_ticks_from_boot() + ms, event = 0;

    while ((!ms || cortex_m_get_ticks_from_boot() < expire) && !event)
    {
#ifdef CONFIG_GPIO
	event |= gpio_events_process();
#endif
#ifdef CONFIG_STELLARIS_ETH
	event |= stellaris_eth_event_process();
#endif
	event |= buffered_serial_events_process();

	MAP_SysCtlSleep();
    }

    return event;
}
