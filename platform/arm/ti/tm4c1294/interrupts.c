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
#include "platform/arm/ti/tm4c1294/tm4c1294.h"

extern void reset_isr(void);
extern void ti_arm_mcu_uart_isr(int u);
void ti_arm_mcu_gpio_isr(int port) { }
extern void cortex_m_systick_isr(void);
#ifdef CONFIG_USB
extern void ti_arm_mcu_usb_isr(void);
#else
static void ti_arm_mcu_usb_isr(void) { }
#endif

static void uart0_isr(void) { ti_arm_mcu_uart_isr(UART0); }
static void uart1_isr(void) { ti_arm_mcu_uart_isr(UART1); }
static void uart2_isr(void) { ti_arm_mcu_uart_isr(UART2); }
static void uart3_isr(void) { ti_arm_mcu_uart_isr(UART3); }
static void uart4_isr(void) { ti_arm_mcu_uart_isr(UART4); }
static void uart5_isr(void) { ti_arm_mcu_uart_isr(UART5); }
static void uart6_isr(void) { ti_arm_mcu_uart_isr(UART6); }
static void uart7_isr(void) { ti_arm_mcu_uart_isr(UART7); }

static void gpio_port_a_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_A); }
static void gpio_port_b_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_B); }
static void gpio_port_c_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_C); }
static void gpio_port_d_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_D); }
static void gpio_port_e_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_E); }
static void gpio_port_f_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_F); }
static void gpio_port_g_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_G); }
static void gpio_port_h_isr(void) { ti_arm_mcu_gpio_isr(GPIO_PORT_H); }

#ifdef CONFIG_TIVA_C_EMAC
extern void tiva_c_emac_isr(void);
#else
#define tiva_c_emac_isr default_isr
#endif

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
    gpio_port_e_isr,                  // GPIO Port E
    uart0_isr,                        // UART0 Rx and Tx
    uart1_isr,                        // UART1 Rx and Tx
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
    gpio_port_f_isr,                  // GPIO Port F
    gpio_port_g_isr,                  // GPIO Port G
    gpio_port_h_isr,                  // GPIO Port H
    uart2_isr,                        // UART2 Rx and Tx
    default_isr,                      // SSI1 Rx and Tx
    default_isr,                      // Timer 3 subtimer A
    default_isr,                      // Timer 3 subtimer B
    default_isr,                      // I2C1 Master and Slave
    default_isr,                      // CAN0
    default_isr,                      // CAN1
    tiva_c_emac_isr,                  // Ethernet
    default_isr,                      // Hibernate
    ti_arm_mcu_usb_isr,               // USB0
    default_isr,                      // PWM Generator 3
    default_isr,                      // uDMA Software Transfer
    default_isr,                      // uDMA Error
    default_isr,                      // ADC1 Sequence 0
    default_isr,                      // ADC1 Sequence 1
    default_isr,                      // ADC1 Sequence 2
    default_isr,                      // ADC1 Sequence 3
    default_isr,                      // External Bus Interface 0
    default_isr,                      // GPIO Port J
    default_isr,                      // GPIO Port K
    default_isr,                      // GPIO Port L
    default_isr,                      // SSI2 Rx and Tx
    default_isr,                      // SSI3 Rx and Tx
    default_isr,                      // UART3 Rx and Tx
    default_isr,                      // UART4 Rx and Tx
    default_isr,                      // UART5 Rx and Tx
    uart3_isr,                        // UART6 Rx and Tx
    uart4_isr,                        // UART7 Rx and Tx
    uart5_isr,                        // I2C2 Master and Slave
    uart6_isr,                        // I2C3 Master and Slave
    uart7_isr,                        // Timer 4 subtimer A
    0,                                // Timer 4 subtimer B
    0,                                // Timer 5 subtimer A
    0,                                // Timer 5 subtimer B
    0,                                // FPU
    default_isr,                      // Reserved
    default_isr,                      // Reserved
    default_isr,                      // I2C4 Master and Slave
    default_isr,                      // I2C5 Master and Slave
    0,                                // GPIO Port M
    0,                                // GPIO Port N
    0,                                // Reserved
    0,                                // Tamper
    0,                                // GPIO Port P (Summary or P0)
    0,                                // GPIO Port P1
    0,                                // GPIO Port P2
    0,                                // GPIO Port P3
    0,                                // GPIO Port P4
    0,                                // GPIO Port P5
    0,                                // GPIO Port P6
    0,                                // GPIO Port P7
    0,                                // GPIO Port Q (Summary or Q0)
    0,                                // GPIO Port Q1
    0,                                // GPIO Port Q2
    0,                                // GPIO Port Q3
    0,                                // GPIO Port Q4
    0,                                // GPIO Port Q5
    0,                                // GPIO Port Q6
    0,                                // GPIO Port Q7
    default_isr,                      // GPIO Port R
    default_isr,                      // GPIO Port S
    default_isr,                      // SHA/MD5 0
    default_isr,                      // AES 0
    default_isr,                      // DES3DES 0
    default_isr,                      // LCD Controller 0
    default_isr,                      // Timer 6 subtimer A
    default_isr,                      // Timer 6 subtimer B
    default_isr,                      // Timer 7 subtimer A
    default_isr,                      // Timer 7 subtimer B
    default_isr,                      // I2C6 Master and Slave
    default_isr,                      // I2C7 Master and Slave
    default_isr,                      // HIM Scan Matrix Keyboard 0
    default_isr,                      // One Wire 0
    default_isr,                      // HIM PS/2 0
    default_isr,                      // HIM LED Sequencer 0
    default_isr,                      // HIM Consumer IR 0
    default_isr,                      // I2C8 Master and Slave
    default_isr,                      // I2C9 Master and Slave
    default_isr,                      // GPIO Port T
};
