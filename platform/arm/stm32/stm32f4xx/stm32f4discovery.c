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
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "util/debug.h"
#include "util/tstr.h"
#include "platform/platform.h"
#include "platform/arm/cortex-m.h"
#include "platform/arm/stm32/stm32f4xx/stm32f4discovery.h"
#include "platform/arm/stm32/stm32_gpio.h"
#include "platform/arm/stm32/stm32_spi.h"
#include "platform/arm/stm32/stm32.h"
#include "drivers/serial/serial_platform.h"

extern uint32_t SystemCoreClock;

const stm32_gpio_port_t stm32_gpio_ports[] = {
    [GPIO_PORT_A] = { RCC_AHB1Periph_GPIOA, GPIOA },
    [GPIO_PORT_B] = { RCC_AHB1Periph_GPIOB, GPIOB },
    [GPIO_PORT_C] = { RCC_AHB1Periph_GPIOC, GPIOC },
    [GPIO_PORT_D] = { RCC_AHB1Periph_GPIOD, GPIOD },
    [GPIO_PORT_E] = { RCC_AHB1Periph_GPIOE, GPIOE },
    [GPIO_PORT_F] = { RCC_AHB1Periph_GPIOF, GPIOF }
};

#ifdef CONFIG_SPI
const stm32_spi_t stm32_spis[] = {
    [SPI_PORT1] = {
	.spix = SPI1,
	.periph_enable = RCC_APB2PeriphClockCmd,
	.spi_clk = RCC_APB2Periph_SPI1,
	.clk = PA5,
	.miso = PA6,
	.mosi = PA7,
	.af = GPIO_AF_SPI1
    },
    [SPI_PORT2] = {
	.spix = SPI2,
	.periph_enable = RCC_APB1PeriphClockCmd,
	.spi_clk = RCC_APB1Periph_SPI2,
	.clk = PB10,
	.miso = PC2,
	.mosi = PC3,
	.af = GPIO_AF_SPI2 
    },
    [SPI_PORT3] = {
	.spix = SPI3,
	.periph_enable = RCC_APB1PeriphClockCmd,
	.spi_clk = RCC_APB1Periph_SPI3,
	.clk = PC10,
	.miso = PC11,
	.mosi = PC12,
	.af = GPIO_AF_SPI3
    }
};
#endif

static void uart_int_enable(int enable)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART2 Receive interrupt: this interrupt is generated when the
     * USART2 receive data register is not empty.
     */
    USART_ITConfig(USART2, USART_IT_RXNE, enable ? ENABLE : DISABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = enable ? ENABLE : DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static int stm32_serial_enable(int u, int enabled)
{
    return 0;
}

static void stm32_serial_irq_enable(int u, int enabled)
{
    uart_int_enable(enabled);
}

static int stm32_serial_write(int u, char *buf, int size)
{
    for (; size--; buf++)
    {
	USART_SendData(USART2, *buf);
	/* Wait until transmit finishes */
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
    return 0;
}

static void timers_init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}

void usart_isr(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == RESET)
	return;

    /* Read a character */
    buffered_serial_push(0, USART_ReceiveData(USART2) & 0x7F);
}

static void usarts_init(void)
{
    USART_InitTypeDef USART_InitStructure;

    /* GPIO Clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    /* USART Clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* Configure USART Tx as alternate function push-pull */
    stm32_gpio_set_pin_function(PA2, GPIO_AF_USART2);
    /* Configure USART Rx as alternate function push-pull */
    stm32_gpio_set_pin_function(PA3, GPIO_AF_USART2);

    /* Initialize USART */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = 
	USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    uart_int_enable(1);

    /* Enable the USART */
    USART_Cmd(USART2, ENABLE);
}

static unsigned long stm32f4discovery_get_system_clock(void)
{
    return SystemCoreClock;
}

static void stm32f4discovery_init(void)
{
    usarts_init();
    timers_init();
}

const platform_t platform = {
    .serial = {
	.enable = stm32_serial_enable,
	.read = buffered_serial_read,
	.write = stm32_serial_write,
	.irq_enable = stm32_serial_irq_enable,
    },
#ifdef CONFIG_GPIO
    .gpio = {
	.digital_write = stm32_gpio_digital_write,
	.set_pin_mode = stm32_gpio_set_pin_mode,
	.set_port_val = stm32_gpio_set_port_val,
    },
#endif
#ifdef CONFIG_SPI
    .spi = {
	.init = stm32_spi_init,
	.reconf = stm32_spi_reconf,
	.set_max_speed = stm32_spi_set_max_speed,
	.send = stm32_spi_send,
	.receive = stm32_spi_receive,
    },
#endif
    .init = stm32f4discovery_init,
    .meminfo = cortex_m_meminfo,
    .panic = cortex_m_panic,
    .select = stm32_select,
    .get_ticks_from_boot = cortex_m_get_ticks_from_boot,
    .get_system_clock = stm32f4discovery_get_system_clock,
    .msleep = stm32_msleep,
};
