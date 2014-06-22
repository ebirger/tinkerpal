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
#include "platform/arm/ti/cc3200/cc3200.h"

extern void reset_isr(void);
extern void ti_arm_mcu_uart_isr(int u);
void ti_arm_mcu_gpio_isr(int port) { }
extern void cortex_m_systick_isr(void);

static void uart0_isr(void) { ti_arm_mcu_uart_isr(UART0); }
static void uart1_isr(void) { ti_arm_mcu_uart_isr(UART1); }
static void gpio_port_a_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_A); }
static void gpio_port_b_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_B); }
static void gpio_port_c_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_C); }
static void gpio_port_d_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_D); }

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
    gpio_port_a_isr,                  // GPIO Port A
    gpio_port_b_isr,                  // GPIO Port B
    gpio_port_c_isr,                  // GPIO Port C
    gpio_port_d_isr,                  // GPIO Port D
    default_isr,                      // Reserved
    uart0_isr,                        // UART0 Rx and Tx
    uart1_isr,                        // UART1 Rx and Tx
    default_isr,                      // Reserved
    default_isr,                      // I2C0 Master and Slave
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // ADC Channel 0
    default_isr,                      // ADC Channel 1
    default_isr,                      // ADC Channel 2
    default_isr,                      // ADC Channel 3
    default_isr,                      // Watchdog Timer
    default_isr,                      // Timer 0 subtimer A
    default_isr,                      // Timer 0 subtimer B
    default_isr,                      // Timer 1 subtimer A
    default_isr,                      // Timer 1 subtimer B
    default_isr,                      // Timer 2 subtimer A
    default_isr,                      // Timer 2 subtimer B
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Flash
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Timer 3 subtimer A
    default_isr,                      // Timer 3 subtimer B
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // uDMA Software Transfer
    default_isr,                      // uDMA Error
    0,                                // Reserved
    0,0,0,0,0,0,0,0,0,                // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    default_isr,                      // SHA
    0,0,                              // Reserved
    default_isr,                      // AES
    0,                                // Reserved
    default_isr,                      // DES
    0,0,0,0,0,                        // Reserved
    default_isr,                      // SDHost
    0,                                // Reserved
    default_isr,                      // I2S
    0,                                // Reserved
    default_isr,                      // Camera
    0,0,0,0,0,0,0,                    // Reserved
    default_isr,                      // NWP to APPS Interrupt
    default_isr,                      // Power, Reset and Clock module
    0,0,                              // Reserved
    default_isr,                      // Shared SPI
    default_isr,                      // Generic SPI
    default_isr,                      // Link SPI
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0,0,0,0,0,0,0,0,0,              // Reserved
    0,0                               // Reserved
};
