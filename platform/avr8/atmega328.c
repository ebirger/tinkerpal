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
#define F_CPU 16000000L
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "platform/platform.h"
#include "platform/ticks.h"
#include "util/tp_misc.h"

#define DEFAULT_BAUD 19200

static void avr8_msleep(double ms)
{
    int i = (int)ms;

    while (i--)
        _delay_ms(1);
}

static void clock_init(void)
{
    /* Set the Timer Mode to CTC */
    TCCR0A |= (1 << WGM01);
    /* Count value */
    OCR0A = 0xF9;
    /* ISR COMPA vector */
    TIMSK0 |= (1 << OCIE0A);
    sei();
    /* Set prescaler to 64 and start the timer */
    TCCR0B |= (1 << CS01) | (1 << CS00);
}

ISR (TIMER0_COMPA_vect)
{
    tick();
}

static void avr8_init(void)
{
    clock_init();
}

static int avr8_select(int ms)
{
    uint64_t expire = platform_get_ticks_from_boot() + ms;
    int event = 0;

    while ((!ms || platform_get_ticks_from_boot() < expire) && !event)
    {
        cli();
        event |= buffered_serial_events_process();
        sei();
    }
    return event;
}


static int avr8_uart_enable(int u, int enabled)
{
    const int ubrr = F_CPU / (DEFAULT_BAUD * 16L) - 1;

    if (!enabled || u != USART0)
        return 0;

    cli();
    /* Set baud rate */
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    /* Set frame format: 8N1 */
    UCSR0C = (3<<UCSZ00);
    /* Enable receiver, xmitter and interrupts */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    /* Turn off Double Speed Operation */
    UCSR0A &= ~(1<<U2X0);
    /* Sleep a while to let it calm down... */
    _delay_ms(1000);
    sei();
    return 0;
}

static int avr8_serial_write(int u, char *buf, int size)
{
    if (u != USART0)
        return 0;

    while (size--)
    {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *buf++;
    }
    return 0;
}

ISR(USART_RX_vect)
{
    buffered_serial_push(USART0, UDR0);
}

static void avr8_serial_irq_enable(int u, int enable)
{
    if (enable)
        sei();
    else
        cli();
}

#define GPIO_SET(reg, pin, val) do { \
    switch (GPIO_PORT(pin)) \
    { \
    case GPIO_PORT_B: bit_set(reg##B, GPIO_BIT(pin), val); break; \
    case GPIO_PORT_C: bit_set(reg##C, GPIO_BIT(pin), val); break; \
    case GPIO_PORT_D: bit_set(reg##D, GPIO_BIT(pin), val); break; \
    } \
} while (0)

static void avr8_gpio_digital_write(int pin, int value)
{
    GPIO_SET(PORT, pin, value);
}

static int avr8_set_pin_mode(int pin, gpio_pin_mode_t mode)
{
    int ddr;

    switch (mode)
    {
    case GPIO_PM_OUTPUT:
        ddr = 1;
        break;
    case GPIO_PM_INPUT:
        ddr = 0;
        break;
    default:
        return -1;
    }

    GPIO_SET(DDR, pin, ddr);
    return 0;
}


const platform_t platform = {
    .serial = {
        .enable = avr8_uart_enable,
        .write = avr8_serial_write,
        .read = buffered_serial_read,
        .irq_enable = avr8_serial_irq_enable,
    },
#ifdef CONFIG_GPIO
    .gpio = {
        .digital_write = avr8_gpio_digital_write,
        .set_pin_mode = avr8_set_pin_mode,
    },
#endif
    .init = avr8_init,
    .select = avr8_select,
    .msleep = avr8_msleep,
    .get_time_from_boot = gen_get_time_from_boot,
};

int main(void)
{
    extern int tp_main(int argc, char *argv[]);

    tp_main(0, 0);
}
