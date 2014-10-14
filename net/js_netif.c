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
#include "net/js_netif.h"
#include "net/net_utils.h"
#include "js/js_utils.h"
#include "js/js_event.h"

#define Snetif_id S("netif_id")

int netif_obj_constructor(netif_t *netif, obj_t **ret, obj_t *this,
    int argc, obj_t *argv[])
{
    *ret = object_new();
    obj_set_property_int(*ret, Snetif_id, netif->id);
    obj_inherit(*ret, argv[0]);
    return 0;
}

netif_t *netif_obj_get_netif(obj_t *o)
{
    int id = -1;
    
    tp_assert(!obj_get_property_int(&id, o, &Snetif_id));
    return netif_get_by_id(id);
}

int do_netif_on_port_change(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);
    event_t *e;

    if (argc != 2)
        return js_invalid_args(ret);

    e = js_event_new(argv[1], this, js_event_gen_trigger);

    netif_on_event_set(netif, NETIF_EVENT_PORT_CHANGE, e);
    *ret = UNDEF;
    return 0;
}

int do_netif_mac_addr_get(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);
    obj_t *array_buffer;
    eth_mac_t *mac;

    array_buffer = array_buffer_new(sizeof(*mac));
    mac = array_buffer_ptr(array_buffer);
    netif_mac_addr_get(netif, mac);
    *ret = array_buffer_view_new(array_buffer,
        ABV_SHIFT_8_BIT | ABV_FLAG_UNSIGNED, 0, sizeof(*mac));
    obj_put(array_buffer);
    return 0;
}

int do_netif_link_status(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);

    *ret = netif_link_status(netif) ? TRUE : FALSE;
    return 0;
}
