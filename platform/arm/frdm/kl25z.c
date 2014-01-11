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
#include "util/debug.h"
#include "platform/platform.h"
#include "platform/arm/cortex-m.h"
#include "platform/arm/frdm/MKL25Z4.h"
#include "drivers/serial/serial_platform.h"

#define CORE_CLOCK 48000000 /* Core clock speed */

/* Serial enable routine (kl25z_serial_enable()) taken from bare-metal-arm, 
 * Copyright (c) 2012-2013 Andrew Payne <andy@payne.org>
 */
static inline void enable_irq(int n) 
{
    NVIC_ICPR |= 1 << (n - 16);
    NVIC_ISER |= 1 << (n - 16);			
}

static int kl25z_serial_enable(int u, int enabled)
{
    int baud_rate = 115200;
    uint16_t divisor;

    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
        
    /* Turn on clock to UART0 module and select 48Mhz clock (FLL/PLL source) */
    SIM_SCGC4 |= SIM_SCGC4_UART0_MASK;
    SIM_SOPT2 &= ~SIM_SOPT2_UART0SRC_MASK;
    SIM_SOPT2 |= SIM_SOPT2_UART0SRC(1); /* FLL/PLL source */

    /* Select "Alt 2" usage to enable UART0 on pins */
    PORTA_PCR1 = PORT_PCR_MUX(2);
    PORTA_PCR2 = PORT_PCR_MUX(2);

    UART0_C2 = 0;
    UART0_C1 = 0;
    UART0_C3 = 0;
    UART0_S2 = 0;     

    /* Set the baud rate divisor */
    #define OVER_SAMPLE 16
    divisor = (CORE_CLOCK / OVER_SAMPLE) / baud_rate;
    UART0_C4 = UARTLP_C4_OSR(OVER_SAMPLE - 1);
    UART0_BDH = (divisor >> 8) & UARTLP_BDH_SBR_MASK;
    UART0_BDL = (divisor & UARTLP_BDL_SBR_MASK);

    /* Enable the transmitter, receiver, and receive interrupts */
    UART0_C2 = UARTLP_C2_RE_MASK | UARTLP_C2_TE_MASK | UART_C2_RIE_MASK;
    enable_irq(INT_UART0);
    return 0;
}

void kl25z_uart_isr(void)
{
    if (!(UART0_S1 & UART_S1_RDRF_MASK))
       return;

    buffered_serial_push(0, UART0_D);
}

static int kl25z_serial_write(int u, char *buf, int size)
{
    while (size-- > 0)
    {
	/* Wait until transmit buffer is empty */
        while (!(UART0_S1 & UART_S1_TDRE_MASK));
        UART0_D = *buf++;
    }
    return 0;
}

static void kl25z_serial_irq_enable(int u, int enabled)
{
    if (enabled)
	UART0_C2 |= UART_C2_RIE_MASK;
    else
	UART0_C2 &= ~UART_C2_RIE_MASK;
}

static int kl25z_select(int ms)
{
    int expire = cortex_m_get_ticks_from_boot() + ms, event = 0;

    while ((!ms || cortex_m_get_ticks_from_boot() < expire) && !event)
    {
	event |= buffered_serial_events_process();

	/* XXX: Sleep */
    }

    return event;
}

/* Clock initialization routine (init_clocks()) taken from bare-metal-arm, 
 * Copyright (c) 2012-2013 Andrew Payne <andy@payne.org>
 * Initialize the system clocks to a 48 Mhz core clock speed
 * Mode progression:  FEI (reset) -> FBE -> PBE -> PEE
 *
 * Note:  Generated by Processor Expert, cleaned up by hand. 
 *        For detailed information on clock modes, see the 
 *        "KL25 Sub-Family Reference Manual" section 24.5.3.1
 */
static void init_clocks(void)
{   
    /* Enable clock gate to Port A module to enable pin routing (PORTA=1) */
    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;
    
    /* Divide-by-2 for clock 1 and clock 4 (OUTDIV1=1, OUTDIV4=1) */
    SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0x01) | SIM_CLKDIV1_OUTDIV4(0x01);

    /* System oscillator drives 32 kHz clock for various peripherals 
     * (OSC32KSEL=0)
     */
    SIM_SOPT1 &= ~SIM_SOPT1_OSC32KSEL(0x03);

    /* Select PLL as a clock source for various peripherals (PLLFLLSEL=1)
     * Clock source for TPM counter clock is MCGFLLCLK or MCGPLLCLK/2
     */
    SIM_SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK;
    SIM_SOPT2 = (SIM_SOPT2 & ~(SIM_SOPT2_TPMSRC(0x02))) | SIM_SOPT2_TPMSRC(0x01);
                  
    /* PORTA_PCR18: ISF=0,MUX=0 */
    /* PORTA_PCR19: ISF=0,MUX=0 */
    PORTA_PCR18 &= ~((PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x07)));
    PORTA_PCR19 &= ~((PORT_PCR_ISF_MASK | PORT_PCR_MUX(0x07)));

    /* Switch to FBE Mode */
    
    /* OSC0_CR: ERCLKEN=0,??=0,EREFSTEN=0,??=0,SC2P=0,SC4P=0,SC8P=0,SC16P=0 */
    OSC0_CR = 0;
    /* MCG_C2: LOCRE0=0,??=0,RANGE0=2,HGO0=0,EREFS0=1,LP=0,IRCS=0 */
    MCG_C2 = (MCG_C2_RANGE0(0x02) | MCG_C2_EREFS0_MASK);
    /* MCG_C1: CLKS=2,FRDIV=3,IREFS=0,IRCLKEN=0,IREFSTEN=0 */
    MCG_C1 = (MCG_C1_CLKS(0x02) | MCG_C1_FRDIV(0x03));
    /* MCG_C4: DMX32=0,DRST_DRS=0 */
    MCG_C4 &= ~((MCG_C4_DMX32_MASK | MCG_C4_DRST_DRS(0x03)));
    /* MCG_C5: ??=0,PLLCLKEN0=0,PLLSTEN0=0,PRDIV0=1 */
    MCG_C5 = MCG_C5_PRDIV0(0x01);
    /* MCG_C6: LOLIE0=0,PLLS=0,CME0=0,VDIV0=0 */
    MCG_C6 = 0;
    
    /* Check that the source of the FLL reference clock is the external 
     * reference clock
     */
    while (MCG_S & MCG_S_IREFST_MASK);

    /* Wait until external reference */
    while ((MCG_S & MCG_S_CLKST_MASK) != 8);
    
    /* Switch to PBE mode, Select PLL as MCG source (PLLS=1) */
    MCG_C6 = MCG_C6_PLLS_MASK;
    /* Wait until PLL locked */
    while (!(MCG_S & MCG_S_LOCK0_MASK));
    
    /* Switch to PEE mode
     *    Select PLL output (CLKS=0)
     *    FLL external reference divider (FRDIV=3)
     *    External reference clock for FLL (IREFS=0)
     */
    MCG_C1 = MCG_C1_FRDIV(0x03);
    /* Wait until PLL output */
    while ((MCG_S & MCG_S_CLKST_MASK) != 0x0CU);
}

static void kl25z_init(void)
{
    init_clocks();
}

const platform_t platform = {
    .serial = {
	.enable = kl25z_serial_enable,
	.read = buffered_serial_read,
	.write = kl25z_serial_write,
	.irq_enable = kl25z_serial_irq_enable,
    },
    .init = kl25z_init,
    .meminfo = cortex_m_meminfo,
    .panic = cortex_m_panic,
    .select = kl25z_select,
    .get_ticks_from_boot = cortex_m_get_ticks_from_boot,
};
