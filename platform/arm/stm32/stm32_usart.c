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
#include "platform/arm/stm32/stm32_usart.h"
#include "platform/arm/stm32/stm32_gpio.h"
#include "drivers/serial/serial_platform.h"

void usart_isr(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == RESET)
	return;

    /* Read a character */
    buffered_serial_push(0, USART_ReceiveData(USART2) & 0x7F);
}

int stm32_usart_enable(int u, int enabled)
{
    USART_InitTypeDef USART_InitStructure;

    /* GPIO Clock */
    stm32_gpio_set_pin_mode(PA2, GPIO_PM_OUTPUT);
    stm32_gpio_set_pin_mode(PA3, GPIO_PM_INPUT);
    /* USART Clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    stm32_gpio_set_pin_function(PA2, STM32_USART_AF);
    /* Configure USART Rx as alternate function push-pull */
    stm32_gpio_set_pin_function(PA3, STM32_USART_AF);

    /* Initialize USART */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = 
	USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    stm32_usart_irq_enable(u, 1);

    /* Enable the USART */
    USART_Cmd(USART2, ENABLE);
    return 0;
}

void stm32_usart_irq_enable(int u, int enabled)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART2 Receive interrupt: this interrupt is generated when the
     * USART2 receive data register is not empty.
     */
    USART_ITConfig(USART2, USART_IT_RXNE, enabled ? ENABLE : DISABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = enabled ? ENABLE : DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

int stm32_usart_write(int u, char *buf, int size)
{
    for (; size--; buf++)
    {
	USART_SendData(USART2, *buf);
	/* Wait until transmit finishes */
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
    return 0;
}
