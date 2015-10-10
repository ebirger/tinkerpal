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
#include "platform/ticks.h"
#include "platform/arm/cortex-m.h"
#include "platform/arm/stm32/stm32f1xx/stm32f1xx_common.h"
#include "platform/arm/stm32/stm32f1xx/stm32f103xx.h"
#include "platform/arm/stm32/stm32_usart.h"
#include "platform/arm/stm32/stm32_gpio.h"
#include "platform/arm/stm32/stm32_spi.h"
#include "platform/arm/stm32/stm32.h"

const stm32_gpio_port_t stm32_gpio_ports[] = {
    [GPIO_PORT_A] = { RCC_APB2Periph_GPIOA, GPIOA },
    [GPIO_PORT_B] = { RCC_APB2Periph_GPIOB, GPIOB },
    [GPIO_PORT_C] = { RCC_APB2Periph_GPIOC, GPIOC },
    [GPIO_PORT_D] = { RCC_APB2Periph_GPIOD, GPIOD },
    [GPIO_PORT_E] = { RCC_APB2Periph_GPIOE, GPIOE },
    [GPIO_PORT_F] = { RCC_APB2Periph_GPIOF, GPIOF },
    [GPIO_PORT_G] = { RCC_APB2Periph_GPIOG, GPIOG }
};

const stm32_usart_t stm32_usarts[] = {
    [USART_PORT1] = {
        .usartx = USART1,
        .periph_enable = RCC_APB2PeriphClockCmd,
        .usart_clk = RCC_APB2Periph_USART1,
        .tx = PA9,
        .rx = PA10,
        .irqn = USART1_IRQn,
    },
    [USART_PORT2] = {
        .usartx = USART2,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .usart_clk = RCC_APB1Periph_USART2,
        .tx = PA2,
        .rx = PA3,
        .irqn = USART2_IRQn,
    },
    [USART_PORT3] = {
        .usartx = USART3,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .usart_clk = RCC_APB1Periph_USART3,
        .tx = PB10,
        .rx = PB11,
        .irqn = USART3_IRQn,
    },
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
        .af = GPIO_Remap_SPI1,
    },
    [SPI_PORT2] = {
        .spix = SPI2,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .spi_clk = RCC_APB1Periph_SPI2,
        .clk = PB13,
        .miso = PB14,
        .mosi = PB15,
        .af = 0 
    },
};
#endif

const platform_t platform = {
    .serial = {
        .enable = stm32_usart_enable,
        .read = buffered_serial_read,
        .write = stm32_usart_write,
        .irq_enable = stm32_usart_irq_enable,
    },
    .mem = {
        .info = cortex_m_meminfo,
    },
#ifdef CONFIG_GPIO
    .gpio = {
        .digital_write = stm32_gpio_digital_write,
        .digital_read = stm32_gpio_digital_read,
        .set_pin_mode = stm32_gpio_set_pin_mode,
        .set_port_val = stm32_gpio_set_port_val,
        .get_port_val = stm32_gpio_get_port_val,
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
    .init = stm32_init,
    .panic = cortex_m_panic,
    .select = stm32_select,
    .get_time_from_boot = gen_get_time_from_boot,
    .get_system_clock = stm32_get_system_clock,
    .msleep = stm32_msleep,
};
