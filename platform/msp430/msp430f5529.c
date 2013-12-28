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
#include "drivers/serial/serial.h"

/* We need to disable the WDT before system start since buffer initialization
 * may take to long.
 */
int _system_pre_init(void)
{
    WDTCTL = WDTPW + WDTHOLD; /* Stop watchdog timer */
    return 1;
}

int main(void)
{
    extern int tp_main(int argc, char *argv[]);

    tp_main(0, 0);
    return 0;
}

static void clock_init(void)
{
    /* Set SMCLK to 12 MHz */
    UCSCTL3 |= SELREF_2; /* Set DCO FLL reference = REFO */
    UCSCTL4 |= SELA_2; /* Set ACLK = REFO */
    __bis_SR_register(SCG0); /* Disable the FLL control loop */
    UCSCTL0 = 0x0000; /* Set lowest possible DCOx, MODx */
    UCSCTL1 = DCORSEL_5; /* Select DCO range 24MHz operation */
    UCSCTL2 = FLLD_1 + 374; /* Set DCO Multiplier for 12MHz */
    /* (N + 1) * FLLRef = Fdco
     * (374 + 1) * 32768 = 12MHz
     * Set FLL Div = fDCOCLK/2
     */
    __bic_SR_register(SCG0); /* Enable the FLL control loop */

    /* Worst-case settling time for the DCO when the DCO range bits have been
     * changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
     * UG for optimization.
     * 32 x 32 x 12 MHz / 32,768 Hz = 375000 = MCLK cycles for DCO to settle
     */
    __delay_cycles(375000);
    /* Loop until XT1,XT2 & DCO fault flag is cleared */
    do
    {
	UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
	/* Clear XT2,XT1,DCO fault flags */
	SFRIFG1 &= ~OFIFG; /* Clear fault flags */
    } while (SFRIFG1&OFIFG); /* Test oscillator fault flag */
}

void msp430f5529_init(void)
{
    clock_init();

    __bis_SR_register(GIE); /* Enable interrupts */
}

typedef struct {
    volatile unsigned char *ctl0;
    volatile unsigned char *ctl1;
    volatile unsigned char *br0;
    volatile unsigned char *br1;
    volatile unsigned char *mctl;
    volatile unsigned char *ie;
    volatile unsigned char *txbuf;
    volatile unsigned char *ifg;
} msp430f5529_uart_t;

static const msp430f5529_uart_t msp430f5529_uarts[] = {
#define U(uca) { \
    .ctl0 = &uca##CTL0, \
    .ctl1 = &uca##CTL1, \
    .br0 = &uca##BR0, \
    .br1 = &uca##BR1, \
    .mctl = &uca##MCTL, \
    .ie = &uca##IE, \
    .txbuf = &uca##TXBUF, \
    .ifg = &uca##IFG, \
}
    [UART0] = U(UCA0),
    [UART1] = U(UCA1),
#undef U
};

int msp430f5529_serial_enable(int u, int enabled)
{
    const msp430f5529_uart_t *uca = &msp430f5529_uarts[u];

    *uca->ctl1 |= UCSWRST; /* Put state machine in reset */
    *uca->ctl0 = 0x00;
    *uca->ctl1 = UCSSEL__SMCLK + UCSWRST; /* Use SMCLK, keep RESET */
    *uca->br0 = 0xe2; /* 9600 Baud */
    *uca->br1 = 0x04;
    *uca->mctl = UCBRF_0; /* Modulation UCBRFx=0 */
    *uca->ctl1 &= ~UCSWRST; /* Initialize USCI state machine */
    *uca->ie &= ~UCTXIE;
    return 0;
}

int msp430f5529_serial_write(int u, char *buf, int size)
{
    const msp430f5529_uart_t *uca = &msp430f5529_uarts[u];

    while (size-- > 0)
    {
	/* Wait until transmit buffer is empty */
	while (!(*uca->ifg & UCTXIFG));
	*uca->txbuf = *buf++;
    }
    return 0;
}

#pragma vector = USCI_A0_VECTOR
__interrupt void uscia0rx_isr(void)
{
    buffered_serial_push(UART0, UCA0RXBUF & 0xff);
}

#pragma vector = USCI_A1_VECTOR
__interrupt void uscia1rx_isr(void)
{
    buffered_serial_push(UART1, UCA1RXBUF & 0xff);
}

void msp430f5529_serial_irq_enable(int u, int enabled)
{
    const msp430f5529_uart_t *uca = &msp430f5529_uarts[u];

    if (enabled)
	*uca->ie |= UCRXIE;
    else
	*uca->ie &= ~UCRXIE;
}

int msp430f5529_select(int ms)
{
    int event = 0;

    while (!event)
    {
	event |= buffered_serial_events_process();

	/* XXX: Sleep */
    }

    return event;
}
