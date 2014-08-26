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
#include "usb_dcd_int.h"
#include "usbd_ioreq.h"
#include "platform/arm/stm32/stm32_gpio.h"
#include "platform/arm/stm32/stm32_usb.h"
#include "platform/platform.h"
#include "util/debug.h"
#include <string.h> /* memcpy */

#define STM32_USB_EVENT_RESET 0x01
#define STM32_USB_EVENT_SETUP 0x02
#define STM32_USB_EVENT_OUT 0x04

#define STM32_USB_STATE_SETUP 0
#define STM32_USB_STATE_DATA 1
#define STM32_USB_STATE_STATUS 2

static USB_OTG_CORE_HANDLE dev;
static unsigned int g_events;
static unsigned int g_state;

static void usb_int_enable(USB_OTG_CORE_HANDLE *pdev, int enable)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = enable ? ENABLE : DISABLE;
    NVIC_Init(&NVIC_InitStructure);
}

int stm32_usb_event_process(void)
{
    unsigned int events;

    usb_int_enable(&dev, 0);
    events = g_events;
    g_events = 0;
    usb_int_enable(&dev, 1);

    if (!events)
        return 0;

    if (events & STM32_USB_EVENT_RESET)
        usbd_event(0, USB_DEVICE_EVENT_RESET);
    if (events & STM32_USB_EVENT_SETUP || events & STM32_USB_EVENT_OUT) 
        usbd_event(USBD_EP0, USB_DEVICE_EVENT_EP_DATA_READY);
    return 1;
}

uint8_t stm32_DataOutStage(USB_OTG_CORE_HANDLE *pdev , uint8_t epnum)
{
    tp_out(("%s\n", __func__));
    if (dev.dev.device_state == USB_OTG_EP0_DATA_OUT)
        g_events |= STM32_USB_EVENT_OUT;
    return USBD_OK;
}

uint8_t stm32_DataInStage(USB_OTG_CORE_HANDLE *pdev , uint8_t epnum)
{
    tp_out(("%s\n", __func__));
    usbd_event(epnum, USB_DEVICE_EVENT_EP_WRITE_ACK);
    if (epnum == 0)
    {
       if (g_state == STM32_USB_STATE_STATUS)
       {
           USBD_CtlReceiveStatus(&dev);
           g_state = STM32_USB_STATE_SETUP;
       }
    }
    return USBD_OK;
}

uint8_t stm32_SetupStage(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    g_events |= STM32_USB_EVENT_SETUP;
    dev.dev.device_state = USB_OTG_EP0_SETUP;
    return USBD_OK;
}

uint8_t stm32_SOF(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    return USBD_OK;
}

uint8_t stm32_Reset(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    g_events |= STM32_USB_EVENT_RESET;
    return USBD_OK;
}

uint8_t stm32_Suspend(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    return USBD_OK;
}

uint8_t stm32_Resume(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    return USBD_OK;
}

uint8_t stm32_IsoINIncomplete(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    return USBD_OK;
}

uint8_t stm32_IsoOUTIncomplete(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    return USBD_OK;
}

uint8_t stm32_DevConnected(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    return USBD_OK;
}

uint8_t stm32_DevDisconnected(USB_OTG_CORE_HANDLE *pdev)
{
    tp_out(("%s\n", __func__));
    return USBD_OK;
}
  
USBD_DCD_INT_cb_TypeDef stm32_DCD_INT_cb = 
{
    .DataOutStage = stm32_DataOutStage,
    .DataInStage = stm32_DataInStage,
    .SetupStage = stm32_SetupStage,
    .SOF = stm32_SOF,
    .Reset = stm32_Reset,
    .Suspend = stm32_Suspend,
    .Resume = stm32_Resume,
    .IsoINIncomplete = stm32_IsoINIncomplete,
    .IsoOUTIncomplete = stm32_IsoOUTIncomplete,
    .DevConnected = stm32_DevConnected,
    .DevDisconnected = stm32_DevDisconnected,
};

USBD_DCD_INT_cb_TypeDef *USBD_DCD_INT_fops = &stm32_DCD_INT_cb;

void stm32_usb_isr(void)
{
    USBD_OTG_ISR_Handler(&dev);
    return;
}

void USB_OTG_BSP_uDelay(const unsigned int usec)
{
    volatile unsigned long wait = 120 * usec / 7;

    while (wait--);
}

void USB_OTG_BSP_mDelay(const unsigned int msec)
{
    platform_msleep(msec);
}

static void usb_board_cfg(USB_OTG_CORE_HANDLE *pdev)
{
    /* XXX: Hard coded for stm32f4 */

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    /* Configure SOF VBUS ID DM DP Pins */
    _stm32_gpio_set_pin_function(PA8, 0, GPIO_AF_OTG1_FS);
    _stm32_gpio_set_pin_function(PA11, 0, GPIO_AF_OTG1_FS);
    _stm32_gpio_set_pin_function(PA12, 0, GPIO_AF_OTG1_FS);
    stm32_gpio_set_pin_mode(PA9, GPIO_PM_INPUT_PULLUP); /* XXX: OD */
    /* this for ID line debug */
    _stm32_gpio_set_pin_function(PA10, 1, GPIO_AF_OTG1_FS);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE);

    RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, ENABLE);
}

static void mask_sof_intr(void)
{
    USB_OTG_GINTMSK_TypeDef intmsk;

    intmsk.d32 = 0;
    intmsk.b.sofintr = 1;  
    USB_OTG_MODIFY_REG32(&dev.regs.GREGS->GINTMSK, intmsk.d32, 0);
}

int stm32_usb_init(void)
{
    tp_out(("STM32 USB Init\n"));
    usb_board_cfg(&dev);
    DCD_Init(&dev, USB_OTG_FS_CORE_ID);
    mask_sof_intr();
    usb_int_enable(&dev, 1);
    return 0;
}

void stm32_usb_connect(void)
{
    tp_out(("STM32 USB Connect\n"));
    DCD_DevConnect(&dev);
}

static uint8_t usb_ep_type_to_stm32_ep_type(usb_ep_type_t type)
{
    switch (type)
    {
    case USB_EP_TYPE_CTRL: return EP_TYPE_CTRL;
    case USB_EP_TYPE_BULK: return EP_TYPE_BULK;
    case USB_EP_TYPE_INTERRUPT: return EP_TYPE_INTR;
    case USB_EP_TYPE_ISOC: return EP_TYPE_ISOC;
    }
    return 0;
}

void stm32_usb_ep_cfg(int ep, int max_pkt_size_in, int max_pkt_size_out,
    usb_ep_type_t type)
{
    uint8_t ep_type;

    tp_out(("STM32 Cfg EP %d\n", ep));

    ep_type = usb_ep_type_to_stm32_ep_type(type);
    if (max_pkt_size_out)
        DCD_EP_Open(&dev, ep, max_pkt_size_out, ep_type);
    if (max_pkt_size_in)
        DCD_EP_Open(&dev, 0x80 | ep, max_pkt_size_in, ep_type);
}

void stm32_usb_ep_data_ack(int ep, int data_phase)
{
    if (ep == USBD_EP0)
    {
        if (data_phase)
            g_state = STM32_USB_STATE_DATA;
        else
            g_state = STM32_USB_STATE_STATUS;
    }
}

void stm32_usb_set_addr(unsigned short addr)
{
    DCD_EP_SetAddress(&dev, (uint8_t)addr);
}

int stm32_usb_ep_data_wait(int ep, unsigned char *data, unsigned long len)
{
    if (ep == USBD_EP0)
    {
        if (g_state == STM32_USB_STATE_DATA)
            return USBD_CtlPrepareRx(&dev, data, len) ? -1 : 0;
    }
    return 0;
}

int stm32_usb_ep_data_get(int ep, unsigned char *data, unsigned long len)
{
    tp_out(("%s: len %d\n", __func__, len));
    if (ep == USBD_EP0 && dev.dev.device_state == USB_OTG_EP0_SETUP)
    {
        memcpy(data, dev.dev.setup_packet, 8);
        dev.dev.device_state = USB_OTG_EP0_IDLE;
        return 8;
    }
    /* Data is already in the buffer provided to stm32_usb_ep_data_wait
     * XXX: need to get correct len
     */
    return len;
}

int stm32_usb_ep_data_send(int ep, unsigned char *data, unsigned long len,
    int last)
{
    int rc = -1;

    tp_out(("%s: buf %p len %d\n", __func__, data, len));
    hexdump(data, len);
    if (ep == USBD_EP0)
    {
        if (last)
            g_state = STM32_USB_STATE_STATUS;
        if (!data)
            rc = USBD_CtlSendStatus(&dev);
        rc = USBD_CtlSendData(&dev, data, len);
    }
    return rc;
}
