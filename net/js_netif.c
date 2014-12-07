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
    obj_t *arr_buf;
    eth_mac_t mac;
    int sz = sizeof(mac);

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    netif_mac_addr_get(netif, &mac);
    arr_buf = array_buffer_new(sz);
    tstr_cpy_buf(&to_array_buffer(arr_buf)->value, (char *)&mac, 0, sz);
    *ret = array_buffer_view_new(arr_buf,
        ABV_SHIFT_8_BIT | ABV_FLAG_UNSIGNED, 0, sz);
    obj_put(arr_buf);
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

        event_watch_set_once(NETIF_RES(netif, NETIF_EVENT_IPV4_CONNECTED), e);
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

int do_netif_tcp_connect(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif;
    event_t *e;
    u32 ip;
    u16 port;
    tstr_t ip_str;

    if (argc != 4)
        return js_invalid_args(ret);

    ip_str = obj_get_str(argv[1]);
    ip = ip_addr_parse(tstr_to_strz(&ip_str), ip_str.len);
    tstr_free(&ip_str);
    if (!ip)
        return js_invalid_args(ret);

    if (!(port = (u16)obj_get_int(argv[2])))
        return js_invalid_args(ret);

    if (!is_function(argv[3]))
        return throw_exception(ret, &S("Invalid callback"));

    if (!(netif = netif_obj_get_netif(this)))
        return throw_exception(ret, &Sinvalid_netif);

    e = js_event_new(argv[3], this, js_event_gen_trigger);

    _event_watch_set(NETIF_RES(netif, NETIF_EVENT_TCP_CONNECTED), e, 0, 1);

    if (netif_tcp_connect(netif, ip, port))
        return throw_exception(ret, &S("Failed to connect"));

    *ret = UNDEF;
    return 0;
}

int do_netif_tcp_disconnect(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    netif_tcp_disconnect(netif);

    *ret = UNDEF;
    return 0;
}

int do_netif_on_tcp_data(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);
    event_t *e;

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    *ret = UNDEF;
    if (argc == 1 || (argc == 2 && argv[1] == UNDEF))
    {
        netif_on_event_clear(netif, NETIF_EVENT_TCP_DATA_AVAIL);
        return 0;
    }

    e = js_event_new(argv[1], this, js_event_gen_trigger);

    netif_on_event_set(netif, NETIF_EVENT_TCP_DATA_AVAIL, e);
    return 0;
}

int do_netif_on_tcp_disconnect(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);
    event_t *e;

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    *ret = UNDEF;
    if (argc == 1 || (argc == 2 && argv[1] == UNDEF))
    {
        netif_on_event_clear(netif, NETIF_EVENT_TCP_DISCONNECTED);
        return 0;
    }

    e = js_event_new(argv[1], this, js_event_gen_trigger);

    _event_watch_set(NETIF_RES(netif, NETIF_EVENT_TCP_DISCONNECTED), e, 0, 1);
    return 0;
}

static int netif_tcp_write_dump(void *ctx, char *buf, int len)
{
    return netif_tcp_write(ctx, buf, len);
}

int do_netif_tcp_write(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);

    if (argc != 2)
        return js_invalid_args(ret);

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    *ret = UNDEF;

    if (is_string(argv[1]))
    {
        string_t *s = to_string(argv[1]);

        __tstr_dump(&s->value, 0, s->value.len, netif_tcp_write_dump, netif);
        return 0;
    }
    
    if (is_num(argv[1]))
    {
	int n = obj_get_int(argv[1]);
	char b;

	if (n < 0 || n > 255)
	    return throw_exception(ret, &S("Value must be in [0-255] range"));

	b = (char)n;
	netif_tcp_write(netif, &b, 1);
	return 0;
    }
    
    if (is_array(argv[1]) || is_array_buffer_view(argv[1]))
    {
	array_iter_t iter;
	int rc = 0;

	array_iter_init(&iter, argv[1], 0);
	while (array_iter_next(&iter))
	{
	    obj_t *new_argv[2];

	    new_argv[0] = argv[0];
	    new_argv[1] = iter.obj;

	    if ((rc = do_netif_tcp_write(ret, this, 2, new_argv)))
		break;
	}
	array_iter_uninit(&iter);
	return rc;
    }

    /* Unknown parameter type */
    return js_invalid_args(ret);
}

static int netif_tcp_read_fill_fn(void *ctx, char *buf, int size)
{
    return netif_tcp_read(ctx, buf, size);
}

int do_netif_tcp_read(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    netif_t *netif = netif_obj_get_netif(this);
    tstr_t data;

    if (argc != 1)
        return js_invalid_args(ret);

    if (!netif)
        return throw_exception(ret, &Sinvalid_netif);

    /* XXX: read as much as possible */
    tstr_alloc(&data, 64);
    data.len = tstr_fill(&data, 64, netif_tcp_read_fill_fn, netif);
    *ret = string_new(data);
    return 0;
}
