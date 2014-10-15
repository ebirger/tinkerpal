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

#define Sinvalid_netif S("Invalid network interface")

static void netif_pointer_free(void *ptr)
{
    netif_free(ptr);
}

int netif_obj_constructor(netif_t *netif, obj_t **ret, obj_t *this,
    int argc, obj_t *argv[])
{
    *ret = pointer_new(netif, netif_pointer_free);
    obj_inherit(*ret, argv[0]);
    return 0;
}

static netif_t *netif_obj_get_netif(obj_t *o)
{
    pointer_t *p;

    if (!is_pointer(o))
        return NULL;

    p = to_pointer(o);
    if (p->free != netif_pointer_free)
        return NULL;

    return p->ptr;
}

int do_netif_on_port_change(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);
    event_t *e;

    if (argc != 2)
        return js_invalid_args(ret);

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

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

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    array_buffer = array_buffer_new(sizeof(*mac));
    mac = array_buffer_ptr(array_buffer);
    netif_mac_addr_get(netif, mac);
    *ret = array_buffer_view_new(array_buffer,
        ABV_SHIFT_8_BIT | ABV_FLAG_UNSIGNED, 0, sizeof(*mac));
    obj_put(array_buffer);
    return 0;
}

int do_netif_ip_addr_get(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);
    tstr_t s;

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    tstr_cpy_str(&s, ip_addr_serialize(netif_ip_addr_get(netif)));
    *ret = string_new(s);
    return 0;
}

int do_netif_ip_connect(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    if (argc > 1)
    {
        event_t *e;

        if (!is_function(argv[1]))
            return throw_exception(ret, &S("Invalid callback"));

        e = js_event_new(argv[1], this, js_event_gen_trigger);

        _event_watch_set(NETIF_RES(netif, NETIF_EVENT_IPV4_CONNECTED), e, 0, 1);
    }

    if (netif_ip_connect(netif))
        return throw_exception(ret, &S("Failed to connect"));

    *ret = UNDEF;
    return 0;
}

int do_netif_ip_disconnect(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    netif_ip_disconnect(netif);

    *ret = UNDEF;
    return 0;
}

int do_netif_link_status(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    *ret = netif_link_status(netif) ? TRUE : FALSE;
    return 0;
}
