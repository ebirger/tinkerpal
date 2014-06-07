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
#include "platform/arm/stm32/stm32_i2c.h"
#include "platform/arm/stm32/stm32_gpio.h"

static inline void wait_for_completion(I2C_TypeDef *i2cx)
{
    while (!I2C_CheckEvent(i2cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
}

void stm32_i2c_reg_write(int port, unsigned char addr, unsigned char reg,
    unsigned char *data, int len)
{
    const stm32_i2c_t *i2c = &stm32_i2cs[port];

    /* Wait until I2C1 is not busy any more */
    while (I2C_GetFlagStatus(i2c->i2cx, I2C_FLAG_BUSY));

    /* Send I2C1 START condition */
    I2C_GenerateSTART(i2c->i2cx, ENABLE);

    /* Wait slave start condition ack */
    while (!I2C_CheckEvent(i2c->i2cx, I2C_EVENT_MASTER_MODE_SELECT));

    /* Send slave address */
    I2C_Send7bitAddress(i2c->i2cx, addr, I2C_Direction_Transmitter);

    /* Wait for I2Cx EV6 */
    while (!I2C_CheckEvent(i2c->i2cx,
	I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
	/* Nothing */
    }

    I2C_SendData(i2c->i2cx, reg);
    while (!I2C_CheckEvent(i2c->i2cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));

    while (len--)
    {
	I2C_SendData(i2c->i2cx, *data++);
	while (!I2C_CheckEvent(i2c->i2cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
    }

    /* Send I2C1 STOP Condition after last byte has been transmitted */
    I2C_GenerateSTOP(i2c->i2cx, ENABLE);
    /* wait for last byte transmitted */
    while (!I2C_CheckEvent(i2c->i2cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

int stm32_i2c_init(int port)
{
    const stm32_i2c_t *i2c = &stm32_i2cs[port];
    I2C_InitTypeDef I2C_InitStructure;

    /* Enable the SPI clock */
    i2c->periph_enable(i2c->clk, ENABLE);

    /* Configure GPIO ports */
    stm32_gpio_set_pin_mode(i2c->scl, GPIO_PM_OUTPUT);
    stm32_gpio_set_pin_mode(i2c->sda, GPIO_PM_OUTPUT);

    if (i2c->af)
    {
        _stm32_gpio_set_pin_function(i2c->scl, 0, i2c->af);
        _stm32_gpio_set_pin_function(i2c->sda, 1, i2c->af);
    }

    /* Configure I2C */
    I2C_InitStructure.I2C_ClockSpeed = 100000; /* 100kHz */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; /* standard 50% */
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Disable; /* XXX: needed? */
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    /* Enable I2C */
    I2C_Init(i2c->i2cx, &I2C_InitStructure);
    I2C_Cmd(i2c->i2cx, ENABLE);
    return 0;
}
