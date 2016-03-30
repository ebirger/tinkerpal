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
#include "platform/msp430/msp430f5529.h"
#include "platform/msp430/msp430f5529_gpio.h"
#include "platform/msp430/msp430f5529_usci.h"

#define PLATFORM_CHIPSET_H "platform/msp430/msp430f5529.chip"

static unsigned char dummy_reg;

#define UCB0MCTL dummy_reg /* No modulation on USCI_B */
#define UCB1MCTL dummy_reg /* No modulation on USCI_B */

const msp430f5529_usci_t msp430f5529_uscis[] = {
#define MSP430_USCI_DEF(id, rx, tx, clk) \
    [USCI##id] = { \
        .ctl0 = &UC##id##CTL0, \
        .ctl1 = &UC##id##CTL1, \
        .br0 = &UC##id##BR0, \
        .br1 = &UC##id##BR1, \
        .mctl = &UC##id##MCTL, \
        .ie = &UC##id##IE, \
        .stat = &UC##id##STAT, \
        .txbuf = &UC##id##TXBUF, \
        .rxbuf = &UC##id##RXBUF, \
        .ifg = &UC##id##IFG, \
        .txpin = tx, \
        .rxpin = rx, \
        .clkpin = clk \
    },

#include "platform/chipset.h"
};

void msp430f5529_usci_set_speed(int port, unsigned long speed)
{
    const msp430f5529_usci_t *usci = &msp430f5529_uscis[port];
    unsigned long presc;

    presc = msp430f5529_get_system_clock() / speed;
    
    *usci->ctl1 |= UCSWRST; /* Put USCI state machine in reset */
    *usci->br0 = presc & 0xff;
    *usci->br1 = (presc >> 8) & 0xff;
    *usci->ctl1 &= ~UCSWRST; /* Release USCI state machine */
}

void msp430f5529_usci_init(int port)
{
    const msp430f5529_usci_t *usci = &msp430f5529_uscis[port];
    
    msp430f5529_gpio_set_pin_mode(usci->rxpin, GPIO_PM_INPUT_PULLUP);
    msp430f5529_gpio_set_pin_function(usci->rxpin, 1);
    msp430f5529_gpio_set_pin_mode(usci->txpin, GPIO_PM_OUTPUT);
    msp430f5529_gpio_set_pin_function(usci->txpin, 1);
    if (usci->clkpin != -1)
    {
        msp430f5529_gpio_set_pin_mode(usci->clkpin, GPIO_PM_OUTPUT);
        msp430f5529_gpio_set_pin_function(usci->clkpin, 1);
    }
}
