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
#include "drivers/lcd/ili93xx_transport.h"
#include "platform/arm/stm32/stm32_gpio.h"
#include "platform/arm/stm32/stm32f1xx/stm32f1xx_common.h"

#define BANK1_C ((uint32_t)0x60020000)
#define BANK1_D ((uint32_t)0x60000000)

static int fsmc_db_init(const ili93xx_db_transport_t *trns)
{
    FSMC_NORSRAMInitTypeDef nor_sram;
    FSMC_NORSRAMTimingInitTypeDef timing = {
	.FSMC_AddressSetupTime = 0x02,
	.FSMC_AddressHoldTime = 0x00,
	.FSMC_DataSetupTime = 0x05,
	.FSMC_BusTurnAroundDuration = 0x00,
	.FSMC_CLKDivision = 0x00,
	.FSMC_DataLatency = 0x00,
	.FSMC_AccessMode = FSMC_AccessMode_B,
    };
    int *p, pins[] = {
	PD14, /* D0 */
	PD15, /* D1 */
	PD0, /* D2 */
	PD1, /* D3 */
	PE7, /* D4 */
	PE8, /* D5 */
	PE9, /* D6 */
	PE10, /* D7 */
	PE11, /* D8 */
	PE12, /* D9 */
	PE13, /* D10 */
	PE14, /* D11 */
	PE15, /* D12 */
	PD8, /* D13 */
	PD9, /* D14 */
	PD10, /* D15 */
	PD4, /* NOE - RD */
	PD5, /* NWE - WR */
	PD7, /* NE1 - CS */
	PD11, /* A16 - RS */
	0
    };

    /* Enable the FSMC Clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

    /* Set pins function */
    for (p = pins; *p; p++)
    {
	stm32_gpio_set_pin_mode(*p, GPIO_PM_OUTPUT);
	stm32_gpio_set_pin_function(*p, 0);
    }

    nor_sram.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    nor_sram.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    nor_sram.FSMC_MemoryType = FSMC_MemoryType_NOR;
    nor_sram.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    nor_sram.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    nor_sram.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    nor_sram.FSMC_WrapMode = FSMC_WrapMode_Disable;
    nor_sram.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    nor_sram.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    nor_sram.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    nor_sram.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    nor_sram.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    nor_sram.FSMC_ReadWriteTimingStruct = &timing;
    nor_sram.FSMC_WriteTimingStruct = &timing;	  

    FSMC_NORSRAMInit(&nor_sram); 

    /* Enable FSMC Bank1_SRAM Bank */
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  
    return 0;
}

static void fsmc_cmd_wr(const ili93xx_db_transport_t *trns, u16 cmd)
{
    *(volatile uint16_t *)BANK1_D = cmd;	
}

static void fsmc_data_wr(const ili93xx_db_transport_t *trns, u16 data)
{
    *(volatile uint16_t *)BANK1_C = data;
}

static unsigned short fsmc_data_rd(const ili93xx_db_transport_t *trns)
{
    return *(volatile uint16_t *)BANK1_C;
}

static ili93xx_db_transport_ops_t stm32_fsmc_ili93xx_trns_ops = {
    .db_init = fsmc_db_init,
    .db_cmd_wr = fsmc_cmd_wr,
    .db_data_wr = fsmc_data_wr,
    .db_data_rd = fsmc_data_rd,
};

ili93xx_db_transport_t stm32_fsmc_ili93xx_trns = {
    .ops = &stm32_fsmc_ili93xx_trns_ops
};
