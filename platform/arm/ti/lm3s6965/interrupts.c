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
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "platform/arm/ti/lm3s6965/lm3s6965.h"

extern void reset_isr(void);
extern void ti_arm_mcu_uart_isr(int u);
extern void gpio_isr(int port);
extern void cortex_m_systick_isr(void);
#ifdef CONFIG_STELLARIS_ETH
extern void stellaris_ethernet_isr(void);
#else
#define stellaris_ethernet_isr default_isr
#endif

static void uart0_isr(void) { ti_arm_mcu_uart_isr(UART0); }

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
    /* Hang in there doing nothing */
    while(1);
}

/* Linker variable that marks the top of the stack */
extern unsigned long _stack_top;

/* The vector table.  Note that the proper constructs must be placed on this to
 * ensure that it ends up at physical address 0x0000.0000.
 */
#ifdef CONFIG_GCC
__attribute__ ((section(".isr_vector")))
#elif defined(CONFIG_TI_CCS5)
#pragma DATA_SECTION(g_pfnVectors, ".intvecs")
#else
#error Compilation environment not set
#endif
void (*const g_pfnVectors[])(void) =
{
    (void (*)(void))((unsigned long) &_stack_top), // The initial stack pointer
    reset_isr,                        // The reset handler
    nmi_isr,                          // The NMI handler
    fault_isr,                        // The hard fault handler
    default_isr,                      // The MPU fault handler
    default_isr,                      // The bus fault handler
    default_isr,                      // The usage fault handler
    0,                                // Reserved
    0,                                // Reserved
    0,                                // Reserved
    0,                                // Reserved
    default_isr,                      // SVCall handler
    default_isr,                      // Debug monitor handler
    0,                                // Reserved
    default_isr,                      // The PendSV handler
    cortex_m_systick_isr,             // The SysTick handler
    default_isr,                      // GPIO Port A
    default_isr,                      // GPIO Port B
    default_isr,                      // GPIO Port C
    default_isr,                      // GPIO Port D
    default_isr,                      // GPIO Port E
    uart0_isr,                        // UART0 Rx and Tx
    default_isr,                      // UART1 Rx and Tx
    default_isr,                      // SSI0 Rx and Tx
    default_isr,                      // I2C0 Master and Slave
    default_isr,                      // PWM Fault
    default_isr,                      // PWM Generator 0
    default_isr,                      // PWM Generator 1
    default_isr,                      // PWM Generator 2
    default_isr,                      // Quadrature Encoder 0
    default_isr,                      // ADC Sequence 0
    default_isr,                      // ADC Sequence 1
    default_isr,                      // ADC Sequence 2
    default_isr,                      // ADC Sequence 3
    default_isr,                      // Watchdog timer
    default_isr,                      // Timer 0 subtimer A
    default_isr,                      // Timer 0 subtimer B
    default_isr,                      // Timer 1 subtimer A
    default_isr,                      // Timer 1 subtimer B
    default_isr,                      // Timer 2 subtimer A
    default_isr,                      // Timer 2 subtimer B
    default_isr,                      // Analog Comparator 0
    default_isr,                      // Analog Comparator 1
    default_isr,                      // Analog Comparator 2
    default_isr,                      // System Control (PLL, OSC, BO)
    default_isr,                      // FLASH Control
    default_isr,                      // GPIO Port F
    default_isr,                      // GPIO Port G
    default_isr,                      // GPIO Port H
    default_isr,                      // UART2 Rx and Tx
    default_isr,                      // SSI1 Rx and Tx
    default_isr,                      // Timer 3 subtimer A
    default_isr,                      // Timer 3 subtimer B
    default_isr,                      // I2C1 Master and Slave
    default_isr,                      // Quadrature Encoder 1
    default_isr,                      // CAN0
    default_isr,                      // CAN1
    default_isr,                      // CAN2
    stellaris_ethernet_isr,           // Ethernet
    default_isr                       // Hibernate
};
