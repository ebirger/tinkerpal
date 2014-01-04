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
#include "platform/msp430/msp430f5529_usci.h"

void msp430f5529_spi_send(int port, unsigned long data)
{
    const msp430f5529_usci_t *usci = &msp430f5529_uscis[port];
    unsigned long dummy;
    unsigned short gie;

    gie = __get_SR_register() & GIE; /* Store current GIE state */
    __disable_interrupt();

    // Clock the actual data transfer and send the bytes. Note that we
    // intentionally not read out the receive buffer during frame transmission
    // in order to optimize transfer speed, however we need to take care of the
    // resulting overrun condition.
    while (!(*usci->ifg & UCTXIFG)); /* Wait while not ready for TX */
    *usci->txbuf = data; /* Write byte */
    while (*usci->stat & UCBUSY); /* Wait for all TX/RX to finish */

    dummy = *usci->rxbuf;
    
    __bis_SR_register(gie); /* Restore original GIE state */
}

unsigned long msp430f5529_spi_receive(int port)
{
    const msp430f5529_usci_t *usci = &msp430f5529_uscis[port];
    unsigned long ret;
    unsigned short gie;

    gie = __get_SR_register() & GIE; /* Store current GIE state */
    __disable_interrupt();

    *usci->ifg &= ~UCRXIFG; /* Ensure RXIFG is clear */

    // Clock the actual data transfer and receive the bytes
    while (!(*usci->ifg & UCTXIFG)); /* Wait while not ready for TX */
    *usci->txbuf = 0xff; /* Write dummy byte */
    while (!(*usci->ifg & UCRXIFG)); // Wait for RX buffer (full)
    ret = *usci->rxbuf;

    __bis_SR_register(gie); /* Restore original GIE state */
    return ret;
}

void msp430f5529_spi_reconf(int port)
{
    const msp430f5529_usci_t *usci = &msp430f5529_uscis[port];

    msp430f5529_usci_init(port);

    /* Initialize USCI for SPI Master operation */
    *usci->ctl1 |= UCSWRST; /* Put USCI state machine in reset */
    /* Mode: 3-pin, 8-bit SPI master, sync */
    *usci->ctl0 = UCCKPH + UCMSB + UCMST + UCMODE_0 + UCSYNC;
    *usci->ctl1 = UCSWRST + UCSSEL_2; /* Use SMCLK, keep RESET */
    *usci->mctl = 0; /* No modulation */
    *usci->ctl1 &= ~UCSWRST; /* Release USCI state machine */
    *usci->ifg &= ~UCRXIFG; /* Clear pending state */
}

int msp430f5529_spi_init(int port)
{
    msp430f5529_spi_reconf(port);
    msp430f5529_usci_set_speed(port, 300000); 
    return 0;
}
