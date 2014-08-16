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
#include "platform/arm/stm32/stm32f4xx/stm32f407xx.h"

extern void reset_isr(void);
extern void usart_isr(int u);
extern void cortex_m_systick_isr(void);
extern void stm32_usb_isr(void);

static void usart1_isr(void) { usart_isr(USART_PORT1); }
static void usart2_isr(void) { usart_isr(USART_PORT2); }
static void usart3_isr(void) { usart_isr(USART_PORT3); }
static void uart4_isr(void) { usart_isr(UART_PORT4); }
static void uart5_isr(void) { usart_isr(UART_PORT5); }
static void usart6_isr(void) { usart_isr(USART_PORT6); }

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
#else
#error Compilation environment not set
#endif
void (*const vector[])(void) =
{
    (void (*)(void))((unsigned long) &_stack_top), /* The initial stack pointer */
    reset_isr, /* The reset handler */
    nmi_isr, /* The NMI handler */
    fault_isr, /* The hard fault handler */
    default_isr, /* MemManage_Handler */
    default_isr, /* BusFault_Handler */
    default_isr, /* UsageFault_Handler */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* SVCall Handler */
    default_isr, /* Debug Monitor Handler */
    default_isr, /* Reserved */
    default_isr, /* PendSV Handler */
    cortex_m_systick_isr, /* SysTick_Handler */
    default_isr, /* Window WatchDog */
    default_isr, /* PVD through EXTI Line detection */
    default_isr, /* Tamper and TimeStamps through the EXTI line */
    default_isr, /* RTC Wakeup through the EXTI line */
    default_isr, /* FLASH */
    default_isr, /* RCC */
    default_isr, /* EXTI Line0 */
    default_isr, /* EXTI Line1 */
    default_isr, /* EXTI Line2 */
    default_isr, /* EXTI Line3 */
    default_isr, /* EXTI Line4 */
    default_isr, /* DMA1 Stream 0 */
    default_isr, /* DMA1 Stream 1 */
    default_isr, /* DMA1 Stream 2 */
    default_isr, /* DMA1 Stream 3 */
    default_isr, /* DMA1 Stream 4 */
    default_isr, /* DMA1 Stream 5 */
    default_isr, /* DMA1 Stream 6 */
    default_isr, /* ADC1 */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* External Line[9:5]s */
    default_isr, /* TIM1 Break and TIM9 */
    default_isr, /* TIM1 Update and TIM10 */
    default_isr, /* TIM1 Trigger and Commutation and TIM11 */
    default_isr, /* TIM1 Capture Compare */
    default_isr, /* TIM2 */
    default_isr, /* TIM3 */
    default_isr, /* TIM4 */
    default_isr, /* I2C1 Event */
    default_isr, /* I2C1 Error */
    default_isr, /* I2C2 Event */
    default_isr, /* I2C2 Error */
    default_isr, /* SPI1 */
    default_isr, /* SPI2 */
    usart1_isr, /* USART1 */
    usart2_isr, /* USART2 */
    usart3_isr, /* USART3 */
    default_isr, /* External Line[15:10]s */
    default_isr, /* RTC Alarm (A and B) through EXTI Line */
    default_isr, /* USB OTG FS Wakeup through EXTI line */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* DMA1 Stream7 */
    default_isr, /* Reserved */
    default_isr, /* SDIO */
    default_isr, /* TIM5 */
    default_isr, /* SPI3 */
    uart4_isr, /* UART4 */
    uart5_isr, /* UART5 */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* DMA2 Stream 0 */
    default_isr, /* DMA2 Stream 1 */
    default_isr, /* DMA2 Stream 2 */
    default_isr, /* DMA2 Stream 3 */
    default_isr, /* DMA2 Stream 4 */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    stm32_usb_isr, /* USB OTG FS */
    default_isr, /* DMA2 Stream 5 */
    default_isr, /* DMA2 Stream 6 */
    default_isr, /* DMA2 Stream 7 */
    usart6_isr, /* USART6 */
    default_isr, /* I2C3 event */
    default_isr, /* I2C3 error */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* Reserved */
    default_isr, /* FPU */
};
