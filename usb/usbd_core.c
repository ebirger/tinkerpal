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
#include "usb/usbd_core_platform.h"
#include "usb/usbd_core.h"
#include "platform/platform.h"
#include "util/tp_misc.h"
#include "util/debug.h"

#ifdef CONFIG_USB_CDC_ACM
#include "usb/cdc_acm_descs.c"
#else
#error No USB device class defined
#endif

#define USB_REQ_GET_STATUS 0
#define USB_REQ_CLEAR_FEATURE 1
#define USB_REQ_SET_FEATURE 3
#define USB_REQ_SET_ADDRESS 5
#define USB_REQ_GET_DESCRIPTOR 6
#define USB_REQ_SET_DESCRIPTOR 7
#define USB_REQ_GET_CONFIGURATION 8
#define USB_REQ_SET_CONFIGURATION 9
#define USB_REQ_GET_INTERFACE 10
#define USB_REQ_SET_INTERFACE 11
#define USB_REQ_SYNCH_FRAME 12

#define USB_REQ_TYPE_STD 0
#define USB_REQ_TYPE_CLASS 1
#define USB_REQ_TYPE_VENDOR 2

typedef enum {
    USBD_STATE_DEFAULT = 0,
    USBD_STATE_ADDR_PENDING = 1,
    USBD_STATE_ADDR = 2,
} usbd_state_t;

#define EP0_SIZE 64
#define NUM_EPS 3

static void handle_setup(int data_len);

static u8 ep0_data[EP0_SIZE];
static usbd_state_t g_state;
static u16 g_addr;

typedef struct {
    int max_pkt_size_out;
    int max_pkt_size_in;

    u8 *send_data;
    u16 send_data_remaining;

    u8 *recv_data;
    u16 recv_data_remaining;
    data_ready_cb_t data_ready_cb;
} usbd_ep_t;

static usbd_ep_t usbd_eps[NUM_EPS];

static void usb_req_def_handler(usb_setup_t *setup)
{
    tp_out(("No handler for bRequest %d\n", setup->bRequest));
}

void usbd_ep_cfg(int ep, int max_pkt_size_in, int max_pkt_size_out)
{
    usbd_eps[ep].max_pkt_size_out = max_pkt_size_out;
    usbd_eps[ep].max_pkt_size_in = max_pkt_size_in;
}

int usbd_ep_send(int ep, u8 *data, int len)
{
    usbd_ep_t *uep = &usbd_eps[ep];

    if (len < uep->max_pkt_size_in)
    {
        uep->send_data_remaining = 0;
        uep->send_data = NULL;
        return platform.usb.ep_data_send(ep, data, len, 1);
    }

    uep->send_data_remaining = len - uep->max_pkt_size_in;
    uep->send_data = data + uep->max_pkt_size_in;
    return platform.usb.ep_data_send(ep, data, uep->max_pkt_size_in, 0);
}

void usbd_ep_wait_for_data(int ep, u8 *data, int len, data_ready_cb_t cb)
{
    usbd_eps[ep].recv_data = data;
    usbd_eps[ep].recv_data_remaining = len;
    usbd_eps[ep].data_ready_cb = cb;
}

static void set_configuration_handler(usb_setup_t *setup)
{
    tp_out(("SET_CONFIGURATION\n"));
    /* Just ack for now */
    platform.usb.ep_data_ack(USBD_EP0, 0);
    usbd_ep_send(USBD_EP0, NULL, 0); /* Status ack */
}

#define MIN(a, b) ((a) > (b) ? (b) : (a))

static void get_descriptor_handler(usb_setup_t *setup)
{
    u16 len;

    if (setup->bmRequestType != 0x80)
        return;

    platform.usb.ep_data_ack(USBD_EP0, 1);

    switch (setup->wValue >> 8)
    {
    case USB_DESC_DEVICE:
        tp_out(("GET_DESCRIPTOR: DEVICE\n"));
        len = MIN(setup->wLength, sizeof(usb_device_desc));
        usbd_ep_send(USBD_EP0, (u8 *)&usb_device_desc, len);
        break;
    case USB_DESC_CONFIGURATION:
        {
            usb_cfg_desc_t *cfg_header = (usb_cfg_desc_t *)&usb_full_cfg_desc;

            tp_out(("GET_DESCRIPTOR: CONFIGURATION\n"));
            /* XXX: validate index, stall if necessary */
            len = MIN(setup->wLength, cfg_header->wTotalLength);
            tp_out(("---------------------------------\n"));
            hexdump((u8 *)&usb_full_cfg_desc, len);
            tp_out(("---------------------------------\n"));
            usbd_ep_send(USBD_EP0, (u8 *)&usb_full_cfg_desc, len);
        }
        break;
    case USB_DESC_STRING:
        {
            const u8 *str = usb_string_descs[setup->wValue & 0xff];

            len = MIN(setup->wLength, str[0]);
            usbd_ep_send(USBD_EP0, (u8 *)str, len);
        }
        break;
    }
}

static void set_addr_handler(usb_setup_t *setup)
{
    if (setup->bmRequestType || setup->wIndex || setup->wLength)
        return;

    if (setup->wValue == 0)
    {
        g_state = USBD_STATE_DEFAULT;
        g_addr = 0;
        return;
    }

    g_addr = setup->wValue;
    tp_out(("Set Address: %d\n", g_addr));
    platform.usb.ep_data_ack(USBD_EP0, 0);
    usbd_ep_send(USBD_EP0, NULL, 0); /* Status ack */
    /* USB spec mandates address can't be set before the end of the status
     * stage.
     */
    g_state = USBD_STATE_ADDR_PENDING;
}

static const usb_req_handler_t std_req_handlers[] = {
    [USB_REQ_GET_STATUS] = usb_req_def_handler,
    [USB_REQ_CLEAR_FEATURE] = usb_req_def_handler,
    [USB_REQ_SET_FEATURE] = usb_req_def_handler,
    [USB_REQ_SET_ADDRESS] = set_addr_handler,
    [USB_REQ_GET_DESCRIPTOR] = get_descriptor_handler,
    [USB_REQ_SET_DESCRIPTOR] = usb_req_def_handler,
    [USB_REQ_GET_CONFIGURATION] = usb_req_def_handler,
    [USB_REQ_SET_CONFIGURATION] = set_configuration_handler,
    [USB_REQ_GET_INTERFACE] = usb_req_def_handler,
    [USB_REQ_SET_INTERFACE] = usb_req_def_handler,
    [USB_REQ_SYNCH_FRAME] = usb_req_def_handler,
};

void usbd_dump_setup(usb_setup_t *setup)
{
#define P(field) tp_out(("%s = %x\n", #field, setup->field))
    P(bmRequestType);
    P(bRequest);
    P(wValue);
    P(wIndex);
    P(wLength);
}

static void handle_setup(int data_len)
{
    usb_setup_t *setup;
    int req_type;

    if (data_len != sizeof(usb_setup_t))
    {
        tp_err(("Invalid setup packet of size %d\n", data_len));
        return;
    }

    setup = (usb_setup_t *)ep0_data;
    req_type = (setup->bmRequestType & ((1<<5)|(1<<6))) >> 5;

    switch (req_type)
    {
    case USB_REQ_TYPE_STD:
        if (setup->bRequest > ARRAY_SIZE(std_req_handlers))
            goto Error;

        std_req_handlers[setup->bRequest](setup);
        break;
    case USB_REQ_TYPE_CLASS:
        usbd_class_req_do(setup);
        break;
    default:
        goto Error;
    }

    return;

Error:
    /* XXX: stall EP0 */
    tp_err(("Unsupported request:\n"));
    usbd_dump_setup(setup);
    return;
}

static void ep_data_recv(int ep)
{
    usbd_ep_t *uep = &usbd_eps[ep];
    data_ready_cb_t cb;
    int len;

    len = MIN(uep->recv_data_remaining, uep->max_pkt_size_out);
    len = platform.usb.ep_data_get(ep, uep->recv_data, len);
    if (len < 0)
    {
        /* XXX: stall, signal upper layer */
        tp_err(("Failed to fetch EP %d data\n", ep));
        return;
    }
    uep->recv_data_remaining -= len;
    if (uep->recv_data_remaining < 0)
    {
        tp_err(("Unexpected data on ep %d\n", ep));
        return;
    }

    cb = uep->data_ready_cb;
    if (ep == USBD_EP0)
    {
        /* On EP0 - default waiting for setup packet */
        usbd_ep_wait_for_data(ep, ep0_data, sizeof(usb_setup_t), handle_setup);
    }
    cb(len);
}

static void ep_write_ack(int ep)
{
    usbd_ep_t *uep = &usbd_eps[ep];

    if (ep == USBD_EP0 && g_state == USBD_STATE_ADDR_PENDING)
    {
        platform.usb.set_addr(g_addr);
        g_state = USBD_STATE_ADDR;
    }
    if (uep->send_data_remaining)
        usbd_ep_send(ep, uep->send_data, uep->send_data_remaining);
}

void usbd_event(int ep, usbd_event_t event)
{
    tp_out(("%s: event %d\n", __FUNCTION__, event));
    switch (event)
    {
    case USB_DEVICE_EVENT_RESET:
        return;
    case USB_DEVICE_EVENT_EP_WRITE_ACK:
        ep_write_ack(ep);
        return;
    case USB_DEVICE_EVENT_EP_DATA_READY:
        ep_data_recv(ep);
        break;
    }
}

void usbd_init(void)
{
    usbd_ep_cfg(USBD_EP0, EP0_SIZE, EP0_SIZE);
    usbd_class_init();
    usbd_ep_wait_for_data(USBD_EP0, ep0_data, sizeof(usb_setup_t),
        handle_setup);
    platform.usb.init();
}
