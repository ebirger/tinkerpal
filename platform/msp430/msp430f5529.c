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

    tp_main(0, NULL);
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

static void msp430f5529_init(void)
{
    clock_init();

    _enable_interrupts();
}

static int msp430f5529_serial_enable(int u, int enabled)
{
    P4SEL = BIT4 + BIT5; /* P4.4,5 = USCI_A1 TXD/RXD */
    P4DIR |= (1<<4); /* Set P4.4 to output direction */
    P5DIR &= ~(1<<5); /* Set P4.5 to input direction */
    UCA1CTL1 |= UCSWRST; /* Put state machine in reset */
    UCA1CTL0 = 0x00;
    UCA1CTL1 = UCSSEL__SMCLK + UCSWRST; /* Use SMCLK, keep RESET */
    UCA1BR0 = 0xe2; /* 9600 Baud */
    UCA1BR1 = 0x04;
    UCA1MCTL = UCBRF_0; /* Modulation UCBRFx=0 */
    UCA1CTL1 &= ~UCSWRST; /* Initialize USCI state machine */
    UCA1IE &= ~UCTXIE;
    return 0;
}

static int msp430f5529_serial_write(int u, char *buf, int size)
{
    while (size-- > 0)
    {
	/* Wait until transmit buffer is empty */
	while (!(UCA1IFG & UCTXIFG));
	UCA1TXBUF = *buf++;
    }
    return 0;
}

#pragma vector = USCI_A1_VECTOR
__interrupt void uscia1rx_isr(void)
{
    buffered_serial_push(0, UCA1RXBUF & 0xff);
}

static void msp430f5529_serial_irq_enable(int u, int enabled)
{
    if (enabled)
	UCA1IE |= UCRXIE;
    else
	UCA1IE &= ~UCRXIE;
}

static int msp430f5529_select(int ms, int (*is_active)(int id), 
    void (*mark_on)(int id))
{
    int event = 0;

    while (!event)
    {
	event |= buffered_serial_events_process(is_active, mark_on);

	/* XXX: Sleep */
    }

    return event;
}

const platform_t platform = {
    .desc = "TI MSP430F5529 USB Experimenter Board",
    .serial = {
	.enable = msp430f5529_serial_enable,
	.read = buffered_serial_read,
	.write = msp430f5529_serial_write,
	.irq_enable = msp430f5529_serial_irq_enable,
	.default_console_id = 0,
    },
    .init = msp430f5529_init,
    .meminfo = NULL,
    .panic = NULL,
    .select = msp430f5529_select,
    .get_ticks_from_boot = NULL,
    .get_system_clock = NULL,
    .msleep = NULL,
};
