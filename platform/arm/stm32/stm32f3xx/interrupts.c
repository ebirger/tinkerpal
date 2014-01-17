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
#include "platform/arm/stm32/stm32f3xx/stm32f3discovery.h"

extern void reset_isr(void);
extern void usart_isr(int u);
extern void cortex_m_systick_isr(void);

static void usart1_isr(void) { usart_isr(USART_PORT1); }
static void usart2_isr(void) { usart_isr(USART_PORT2); }
static void usart3_isr(void) { usart_isr(USART_PORT3); }
static void uart4_isr(void) { usart_isr(UART_PORT4); }
static void uart5_isr(void) { usart_isr(UART_PORT5); }

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
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* SVC_Handler */
    default_isr, /* DebugMon_Handler */
    default_isr, /* 0 */
    default_isr, /* PendSV_Handler */
    cortex_m_systick_isr, /* SysTick_Handler */
    default_isr, /* WWDG_IRQHandler */
    default_isr, /* PVD_IRQHandler */
    default_isr, /* TAMPER_STAMP_IRQHandler */
    default_isr, /* RTC_WKUP_IRQHandler */
    default_isr, /* FLASH_IRQHandler */
    default_isr, /* RCC_IRQHandler */
    default_isr, /* EXTI0_IRQHandler */
    default_isr, /* EXTI1_IRQHandler */
    default_isr, /* EXTI2_TS_IRQHandler */
    default_isr, /* EXTI3_IRQHandler */
    default_isr, /* EXTI4_IRQHandler */
    default_isr, /* DMA1_Channel1_IRQHandler */
    default_isr, /* DMA1_Channel2_IRQHandler */
    default_isr, /* DMA1_Channel3_IRQHandler */
    default_isr, /* DMA1_Channel4_IRQHandler */
    default_isr, /* DMA1_Channel5_IRQHandler */
    default_isr, /* DMA1_Channel6_IRQHandler */
    default_isr, /* DMA1_Channel7_IRQHandler */
    default_isr, /* ADC1_2_IRQHandler */
    default_isr, /* USB_HP_CAN1_TX_IRQHandler */
    default_isr, /* USB_LP_CAN1_RX0_IRQHandler */
    default_isr, /* CAN1_RX1_IRQHandler */
    default_isr, /* CAN1_SCE_IRQHandler */
    default_isr, /* EXTI9_5_IRQHandler */
    default_isr, /* TIM1_BRK_TIM15_IRQHandler */
    default_isr, /* TIM1_UP_TIM16_IRQHandler */
    default_isr, /* TIM1_TRG_COM_TIM17_IRQHandler */
    default_isr, /* TIM1_CC_IRQHandler */
    default_isr, /* TIM2_IRQHandler */
    default_isr, /* TIM3_IRQHandler */
    default_isr, /* TIM4_IRQHandler */
    default_isr, /* I2C1_EV_IRQHandler */
    default_isr, /* I2C1_ER_IRQHandler */
    default_isr, /* I2C2_EV_IRQHandler */
    default_isr, /* I2C2_ER_IRQHandler */
    default_isr, /* SPI1_IRQHandler */
    default_isr, /* SPI2_IRQHandler */
    usart1_isr, /* USART1_IRQHandler */
    usart2_isr, /* USART2_IRQHandler */
    usart3_isr, /* USART3_IRQHandler */
    default_isr, /* EXTI15_10_IRQHandler */
    default_isr, /* RTC_Alarm_IRQHandler */
    default_isr, /* USBWakeUp_IRQHandler */
    default_isr, /* TIM8_BRK_IRQHandler */
    default_isr, /* TIM8_UP_IRQHandler */
    default_isr, /* TIM8_TRG_COM_IRQHandler */
    default_isr, /* TIM8_CC_IRQHandler */
    default_isr, /* ADC3_IRQHandler */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* SPI3_IRQHandler */
    uart4_isr, /* UART4_IRQHandler */
    uart5_isr, /* UART5_IRQHandler */
    default_isr, /* TIM6_DAC_IRQHandler */
    default_isr, /* TIM7_IRQHandler */
    default_isr, /* DMA2_Channel1_IRQHandler */
    default_isr, /* DMA2_Channel2_IRQHandler */
    default_isr, /* DMA2_Channel3_IRQHandler */
    default_isr, /* DMA2_Channel4_IRQHandler */
    default_isr, /* DMA2_Channel5_IRQHandler */
    default_isr, /* ADC4_IRQHandler */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* COMP1_2_3_IRQHandler */
    default_isr, /* COMP4_5_6_IRQHandler */
    default_isr, /* COMP7_IRQHandler */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* USB_HP_IRQHandler */
    default_isr, /* USB_LP_IRQHandler */
    default_isr, /* USBWakeUp_RMP_IRQHandler */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* 0 */
    default_isr, /* FPU_IRQHandler */
};
