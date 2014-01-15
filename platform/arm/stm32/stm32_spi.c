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
#include "platform/arm/stm32/stm32_spi.h"
#include "platform/arm/stm32/stm32_gpio.h"

void stm32_spi_set_max_speed(int port, unsigned long speed)
{
}

void stm32_spi_send(int port, unsigned long data)
{
    SPI_TypeDef *spix = stm32_spis[port].spix;

    /* Wait for TX to clear */
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_TXE) == RESET);

    SPI_I2S_SendData(spix, (uint16_t)data);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == RESET);

    SPI_I2S_ReceiveData(spix);
}

unsigned long stm32_spi_receive(int port)
{
    SPI_TypeDef *spix = stm32_spis[port].spix;

    /* Wait for TX to clear */
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_TXE) == RESET);

    SPI_I2S_SendData(spix, 0xff /* dummy */);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(spix, SPI_I2S_FLAG_RXNE) == RESET);

    return SPI_I2S_ReceiveData(spix);
}

void stm32_spi_reconf(int port)
{
    const stm32_spi_t *spi = &stm32_spis[port];
    SPI_InitTypeDef SPI_InitStructure;

    /* Enable the SPI clock */
    spi->periph_enable(spi->spi_clk, ENABLE);

    /* Configure GPIO ports */
    stm32_gpio_set_pin_mode(spi->clk, GPIO_PM_OUTPUT);
    stm32_gpio_set_pin_mode(spi->mosi, GPIO_PM_OUTPUT);
    stm32_gpio_set_pin_mode(spi->miso, GPIO_PM_INPUT);

    stm32_gpio_set_pin_function(spi->clk, spi->af);
    stm32_gpio_set_pin_function(spi->mosi, spi->af);
    stm32_gpio_set_pin_function(spi->miso, spi->af);

    /* Configure SPI */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    /* XXX: calculate prescaler */
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;

    /* Enable SPI  */
    SPI_Init(spi->spix, &SPI_InitStructure);
    SPI_Cmd(spi->spix, ENABLE);
}

int stm32_spi_init(int port)
{
    stm32_spi_reconf(port);
    stm32_spi_set_max_speed(port, 400000); 
    return 0;
}
