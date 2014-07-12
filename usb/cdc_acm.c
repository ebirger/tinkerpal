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
#include "platform/platform.h"
#include "usb/usbd_core.h"
#include "usb/usb_descs.h"
#include "util/debug.h"

#define CDC_ACM_REQ_SET_LINE_CODING 0x20
#define CDC_ACM_REQ_GET_LINE_CODING 0x21
#define CDC_ACM_REQ_SET_CONTROL_LINE_STATE 0x22

typedef struct __packed {
    u32 dwDTERate;
    u8 bCharFormat;
    u8 bParityType;
    u8 bDataBits;
} cdc_acm_line_coding_t;

static cdc_acm_line_coding_t g_line_coding = {
    .dwDTERate = 115200,
    .bCharFormat = 0,
    .bParityType = 0,
    .bDataBits = 8
};

static void data_ready(void)
{
    tp_out(("Data ready\n"));
#define PLE(field) tp_out(("%s = %d\n", #field, g_line_coding.field))
    PLE(dwDTERate);
    PLE(bCharFormat);
    PLE(bParityType);
    PLE(bDataBits);
    platform.usb.ep_data_ack(USBD_EP0, 0);
}

static void set_line_coding_handler(usb_setup_t *setup)
{
    tp_out(("CDC ACM: SET_LINE_CODING\n"));

    if (setup->wValue != 0 || setup->wIndex != 0 || setup->wLength != 7)
    {
        /* XXX: stall */
        tp_err(("Malformed request\n"));
        return;
    }

    usbd_ep_wait_for_data(USBD_EP0, (u8 *)&g_line_coding, 7, data_ready);
    platform.usb.ep_data_ack(USBD_EP0, 1);
}

static void get_line_coding_handler(usb_setup_t *setup)
{
    tp_out(("CDC ACM: GET_LINE_CODING\n"));

    if (setup->wValue != 0 || setup->wIndex != 0 || setup->wLength != 7)
    {
        /* XXX: stall */
        tp_err(("Malformed request\n"));
        platform.usb.ep_data_ack(USBD_EP0, 0);
        return;
    }

    platform.usb.ep_data_ack(USBD_EP0, 1);
    usbd_ep0_send((u8 *)&g_line_coding, 7);
}

static void set_ctrl_line_state_handler(usb_setup_t *setup)
{
    tp_out(("CDC ACM: SET_CONTROL_LINE_STATE\n"));

    platform.usb.ep_data_ack(USBD_EP0, 0);
    /* Do nothing for now */
}

void usbd_class_req_do(usb_setup_t *setup)
{
    switch (setup->bRequest)
    {
    case CDC_ACM_REQ_SET_LINE_CODING:
        set_line_coding_handler(setup);
        break;
    case CDC_ACM_REQ_GET_LINE_CODING:
        get_line_coding_handler(setup);
        break;
    case CDC_ACM_REQ_SET_CONTROL_LINE_STATE:
        set_ctrl_line_state_handler(setup);
        break;
    default:
        tp_out(("Unknown class request\n"));
        usbd_dump_setup(setup);
        break;
    }
}

void usbd_class_init(void)
{
    usbd_ep_cfg(USBD_EP1, 0x10);
    usbd_ep_cfg(USBD_EP2, 0x40);
}
