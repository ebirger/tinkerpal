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
#include <msp430.h>
#include "platform/platform.h"
#include "platform/msp430/msp430f5529_gpio.h"
#include "platform/msp430/msp430f5529.h"
#include "drivers/serial/serial.h"

int msp430f5529_exp_serial_enable(int u, int enabled)
{
    P4SEL = BIT4 + BIT5; /* P4.4,5 = USCI_A1 TXD/RXD */
    P4DIR |= (1<<4); /* Set P4.4 to output direction */
    P5DIR &= ~(1<<5); /* Set P4.5 to input direction */
    return msp430f5529_serial_enable(u, enabled);
}

const platform_t platform = {
    .desc = "TI MSP430F5529 USB Experimenter Board",
    .serial = {
	.enable = msp430f5529_exp_serial_enable,
	.read = buffered_serial_read,
	.write = msp430f5529_serial_write,
	.irq_enable = msp430f5529_serial_irq_enable,
	.default_console_id = UART1,
    },
#ifdef CONFIG_GPIO
    .gpio = {
	.digital_write = msp430f5529_gpio_digital_write,
	.digital_read = msp430f5529_gpio_digital_read,
	.analog_write = NULL,
	.analog_read = NULL,
	.set_pin_mode = msp430f5529_gpio_set_pin_mode,
    },
#endif
    .init = msp430f5529_init,
    .meminfo = NULL,
    .panic = NULL,
    .select = msp430f5529_select,
    .get_ticks_from_boot = NULL,
    .get_system_clock = NULL,
    .msleep = NULL,
};