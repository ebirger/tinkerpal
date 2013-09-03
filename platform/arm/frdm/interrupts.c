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

#include "platform/arm/cortex-m.h"
#include "platform/arm/frdm/MKL25Z4.h"

extern void reset_isr(void);
extern void cortex_m_systick_isr(void);

static void nmi_isr(void)
{
    /* Hang in there doing nothing */
    while(1);
}

static void fault_isr(void)
{
    /* Hang in there doing nothing */
    while(1);
}

static void default_isr(void)
{
    __asm("bkpt");
}

static void (*const g_pfnVectors[])(void);

void reset_isr(void)
{
    extern int tp_main(int argc, char *argv[]);

    SIM_COPC = 0; /* Disable watchdog timer */
    SCB_VTOR = (uint32_t)g_pfnVectors; /* ISR vector position */

    cortex_m_reset_isr();

    tp_main(0, (char **)0);
}

/* Linker variable that marks the top of the stack */
extern unsigned long _stack_top;

/* The vector table.  Note that the proper constructs must be placed on this to
 * ensure that it ends up at physical address 0x0000.0000.
 */
#ifdef CONFIG_GCC
__attribute__ ((section(".isr_vector")))
#else
#error Compilation environment not set
#endif
static void (*const g_pfnVectors[])(void) =
{
    (void (*)(void))((unsigned long) &_stack_top), /* The initial stack pointer */
    reset_isr, /* The reset handler */
    nmi_isr, /* The NMI handler */
    fault_isr, /* The hard fault handler */
    default_isr, /* The MPU fault handler */
    default_isr, /* The bus fault handler */
    default_isr, /* The usage fault handler */
    0, /* Reserved */
    0, /* Reserved */
    0, /* Reserved */
    0, /* Reserved */
    default_isr, /* SVCall handler */
    default_isr, /* Debug monitor handler */
    0, /* Reserved */
    default_isr, /* The PendSV handler */
    cortex_m_systick_isr, /* The SysTick handler */

    /* Interrupts */
    default_isr, /* DMA Channel 0 Transfer Complete and Error */
    default_isr, /* DMA Channel 1 Transfer Complete and Error */
    default_isr, /* DMA Channel 2 Transfer Complete and Error */
    default_isr, /* DMA Channel 3 Transfer Complete and Error */
    default_isr, /* Normal Interrupt */
    default_isr, /* FTFL Interrupt */
    default_isr, /* PMC Interrupt */
    default_isr, /* Low Leakage Wake-up */
    default_isr, /* I2C0 interrupt */
    default_isr, /* I2C1 interrupt */
    default_isr, /* SPI0 Interrupt */
    default_isr, /* SPI1 Interrupt */
    default_isr, /* UART0 Status and Error interrupt */
    default_isr, /* UART1 Status and Error interrupt */
    default_isr, /* UART2 Status and Error interrupt */
    default_isr, /* ADC0 interrupt */
    default_isr, /* CMP0 interrupt */
    default_isr, /* FTM0 fault, overflow and channels interrupt */
    default_isr, /* FTM1 fault, overflow and channels interrupt */
    default_isr, /* FTM2 fault, overflow and channels interrupt */
    default_isr, /* RTC Alarm interrupt */
    default_isr, /* RTC Seconds interrupt */
    default_isr, /* PIT timer all channels interrupt */
    default_isr, /* Reserved interrupt 39/23 */
    default_isr, /* USB interrupt */
    default_isr, /* DAC0 interrupt */
    default_isr, /* TSI0 Interrupt */
    default_isr, /* MCG Interrupt */
    default_isr, /* LPTimer interrupt */
    default_isr, /* Reserved interrupt 45/29 */
    default_isr, /* Port A interrupt */
    default_isr, /* Port D interrupt */
};
