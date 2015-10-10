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
#include "platform/arm/stm32/stm32f4xx/stm32f4xx_common.h"
#include "platform/arm/stm32/stm32f4xx/stm32f407xx.h"
#include "platform/arm/stm32/stm32_usart.h"
#include "platform/arm/stm32/stm32_gpio.h"
#include "platform/arm/stm32/stm32_spi.h"
#include "platform/arm/stm32/stm32_i2c.h"
#include "platform/arm/stm32/stm32_usb.h"
#include "platform/arm/stm32/stm32.h"

const stm32_gpio_port_t stm32_gpio_ports[] = {
    [GPIO_PORT_A] = { RCC_AHB1Periph_GPIOA, GPIOA },
    [GPIO_PORT_B] = { RCC_AHB1Periph_GPIOB, GPIOB },
    [GPIO_PORT_C] = { RCC_AHB1Periph_GPIOC, GPIOC },
    [GPIO_PORT_D] = { RCC_AHB1Periph_GPIOD, GPIOD },
    [GPIO_PORT_E] = { RCC_AHB1Periph_GPIOE, GPIOE },
    [GPIO_PORT_F] = { RCC_AHB1Periph_GPIOF, GPIOF }
};

const stm32_usart_t stm32_usarts[] = {
    [USART_PORT1] = {
        .usartx = USART1,
        .periph_enable = RCC_APB2PeriphClockCmd,
        .usart_clk = RCC_APB2Periph_USART1,
        .tx = PA9,
        .rx = PA10,
        .af = GPIO_AF_USART1,
        .irqn = USART1_IRQn,
    },
    [USART_PORT2] = {
        .usartx = USART2,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .usart_clk = RCC_APB1Periph_USART2,
        .tx = PA2,
        .rx = PA3,
        .af = GPIO_AF_USART2,
        .irqn = USART2_IRQn,
    },
    [USART_PORT3] = {
        .usartx = USART3,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .usart_clk = RCC_APB1Periph_USART3,
        .tx = PB10,
        .rx = PB11,
        .af = GPIO_AF_USART3,
        .irqn = USART3_IRQn,
    },
    [UART_PORT4] = {
        .usartx = UART4,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .usart_clk = RCC_APB1Periph_UART4,
        .tx = PA0,
        .rx = PA1,
        .af = GPIO_AF_UART4,
        .irqn = UART4_IRQn,
    },
    [UART_PORT5] = {
        .usartx = UART5,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .usart_clk = RCC_APB1Periph_UART5,
        .tx = PC12,
        .rx = PD2,
        .af = GPIO_AF_UART5,
        .irqn = UART5_IRQn,
    },
    [USART_PORT6] = {
        .usartx = USART6,
        .periph_enable = RCC_APB2PeriphClockCmd,
        .usart_clk = RCC_APB2Periph_USART6,
        .tx = PC6,
        .rx = PC7,
        .af = GPIO_AF_USART6,
        .irqn = USART6_IRQn,
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

#ifdef CONFIG_I2C
const stm32_i2c_t stm32_i2cs[] = {
    [I2C_PORT1] = {
        .i2cx = I2C1,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .clk = RCC_APB1Periph_I2C1,
        .scl = PB8,
        .sda = PB9,
        .af = GPIO_AF_I2C1,
    },
    [I2C_PORT2] = {
        .i2cx = I2C2,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .clk = RCC_APB1Periph_I2C2,
        .scl = PB10,
        .sda = PB11,
        .af = GPIO_AF_I2C2,
    },
    [I2C_PORT3] = {
        .i2cx = I2C3,
        .periph_enable = RCC_APB1PeriphClockCmd,
        .clk = RCC_APB1Periph_I2C3,
        .scl = PA8,
        .sda = PC9,
        .af = GPIO_AF_I2C3,
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
#ifdef CONFIG_I2C
    .i2c = {
        .init = stm32_i2c_init,
        .reg_write = stm32_i2c_reg_write,
    },
#endif
#ifdef CONFIG_USB_DEVICE
    .usb = {
        .init = stm32_usb_init,
        .connect = stm32_usb_connect,
        .ep_cfg = stm32_usb_ep_cfg,
        .ep_data_ack = stm32_usb_ep_data_ack,
        .ep_data_wait = stm32_usb_ep_data_wait,
        .ep_data_get = stm32_usb_ep_data_get,
        .ep_data_send = stm32_usb_ep_data_send,
        .set_addr = stm32_usb_set_addr,
    },
#endif
    .init = stm32_init,
    .panic = cortex_m_panic,
    .select = stm32_select,
    .get_time_from_boot = gen_get_time_from_boot,
    .get_system_clock = stm32_get_system_clock,
    .msleep = stm32_msleep,
};
