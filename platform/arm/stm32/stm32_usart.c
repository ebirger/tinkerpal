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

void usart_isr(int u)
{
    const stm32_usart_t *usart = &stm32_usarts[u];

    if (USART_GetITStatus(usart->usartx, USART_IT_RXNE) == RESET)
	return;

    USART_ClearITPendingBit(usart->usartx, USART_IT_RXNE);

    /* Read a character */
    buffered_serial_push(u, USART_ReceiveData(usart->usartx) & 0x7F);
}

int stm32_usart_enable(int u, int enabled)
{
    const stm32_usart_t *usart = &stm32_usarts[u];
    USART_InitTypeDef USART_InitStructure;
    USART_ClockInitTypeDef USART_ClockInitStructure;

    /* XXX: support usart disable */
    if (!enabled)
	return 0;

    /* GPIO Clock */
    stm32_gpio_set_pin_mode(usart->tx, GPIO_PM_OUTPUT);
    stm32_gpio_set_pin_mode(usart->rx, GPIO_PM_INPUT);

    /* USART Clock */
    usart->periph_enable(usart->usart_clk, ENABLE);
    USART_ClockStructInit(&USART_ClockInitStructure);
    USART_ClockInit(usart->usartx, &USART_ClockInitStructure);

    /* Configure USART Tx as alternate function push-pull */
    stm32_gpio_set_pin_function(usart->tx, usart->af);
    /* Configure USART Rx as alternate function push-pull */
    stm32_gpio_set_pin_function(usart->rx, usart->af);

    /* Initialize USART */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = 
	USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(usart->usartx, &USART_InitStructure);

    stm32_usart_irq_enable(u, 1);

    /* Enable the USART */
    USART_Cmd(usart->usartx, ENABLE);
    return 0;
}

void stm32_usart_irq_enable(int u, int enabled)
{
    const stm32_usart_t *usart = &stm32_usarts[u];
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = usart->irqn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = enabled ? ENABLE : DISABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Enable the USART Receive interrupt: this interrupt is generated when the
     * USART receive data register is not empty.
     */
    USART_ClearITPendingBit(usart->usartx, USART_IT_RXNE);
    USART_ITConfig(usart->usartx, USART_IT_RXNE, enabled ? ENABLE : DISABLE);

}

int stm32_usart_write(int u, char *buf, int size)
{
    const stm32_usart_t *usart = &stm32_usarts[u];

    for (; size--; buf++)
    {
	USART_SendData(usart->usartx, *buf);
	/* Wait until transmit finishes */
	while (USART_GetFlagStatus(usart->usartx, USART_FLAG_TXE) == RESET);
    }
    return 0;
}
