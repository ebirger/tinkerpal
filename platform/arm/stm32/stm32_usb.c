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
#include "usb_core.h"
#include "usb_defines.h"
#include "usb_dcd.h"
#include "platform/arm/stm32/stm32_gpio.h"
#include "platform/arm/stm32/stm32_usb.h"
#include "platform/platform.h"
#include "util/debug.h"

static USB_OTG_CORE_HANDLE dev;

void stm32_usb_isr(void)
{
    tp_out(("STM32 USB ISR\n"));
}

void USB_OTG_BSP_uDelay(const unsigned int usec)
{
    volatile unsigned int count = 0;
    const unsigned int utime = 120 * usec / 7;

    while (count++ < utime);
}

void USB_OTG_BSP_mDelay (const uint32_t msec)
{
    platform_msleep(msec);
}

static void usb_board_cfg(USB_OTG_CORE_HANDLE *pdev)
{
    /* XXX: Hard coded for stm32f4 */

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
    /* Configure SOF VBUS ID DM DP Pins */
    _stm32_gpio_set_pin_function(PA8, 0, GPIO_AF_OTG1_FS);
    _stm32_gpio_set_pin_function(PA9, 0, GPIO_AF_OTG1_FS);
    _stm32_gpio_set_pin_function(PA11, 0, GPIO_AF_OTG1_FS);
    _stm32_gpio_set_pin_function(PA12, 0, GPIO_AF_OTG1_FS);
    /* this for ID line debug */
    _stm32_gpio_set_pin_function(PA10, 1, GPIO_AF_OTG1_FS);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, ENABLE);
}

static void usb_int_enable(USB_OTG_CORE_HANDLE *pdev)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

int stm32_usb_init(void)
{
    tp_out(("STM32 USB Init\n"));
    usb_board_cfg(&dev);
    DCD_Init(&dev, USB_OTG_FS_CORE_ID);
    usb_int_enable(&dev);
    return 0;
}

void stm32_usb_connect(void)
{
    tp_out(("STM32 USB Connect\n"));
    DCD_DevConnect(&dev);
}

void stm32_usb_ep_cfg(int ep, int max_pkt_size_in, int max_pkt_size_out,
    usb_ep_type_t type)
{
}

void stm32_usb_ep_data_ack(int ep, int data_phase)
{
}

void stm32_usb_set_addr(unsigned short addr)
{
    DCD_EP_SetAddress(&dev, (uint8_t)addr);
}

int stm32_usb_ep_data_get(int ep, unsigned char *data, unsigned long len)
{
    return -1;
}

int stm32_usb_ep_data_send(int ep, unsigned char *data, unsigned long len,
    int last)
{
    return -1;
}
