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
#include <stdio.h> /* NULL */
#include "util/debug.h"
#include "net/netif.h"
#include "net/net.h"

etherif_t *etherif_get_by_id(int id)
{
    return netif_to_etherif(netif_get_by_id(id));
}

static void etherif_netif_mac_addr_get(netif_t *netif, eth_mac_t *mac)
{
    etherif_mac_addr_get(netif_to_etherif(netif), mac);
}

static int etherif_netif_link_status(netif_t *netif)
{
    return etherif_link_status(netif_to_etherif(netif));
}

static int etherif_netif_ip_connect(netif_t *netif)
{
#ifdef CONFIG_DHCP_CLIENT
    return dhcpc_start(netif_to_etherif(netif));
#else
    tp_warn(("etherif: no connect method available\n"));
    return 0;
#endif
}

static void etherif_netif_ip_disconnect(netif_t *netif)
{
#ifdef CONFIG_DHCP_CLIENT
    dhcpc_stop(netif_to_etherif(netif));
#endif
}

static u32 etherif_netif_ip_addr_get(netif_t *netif)
{
    return ipv4_addr(netif_to_etherif(netif));
}

static void etherif_netif_free(netif_t *netif)
{
    etherif_free(netif_to_etherif(netif));
}

static const netif_ops_t etherif_netif_ops = {
    .mac_addr_get = etherif_netif_mac_addr_get,
    .link_status = etherif_netif_link_status,
    .ip_connect = etherif_netif_ip_connect,
    .ip_disconnect = etherif_netif_ip_disconnect,
    .ip_addr_get = etherif_netif_ip_addr_get,
    .free = etherif_netif_free,
};

etherif_t *netif_to_etherif(netif_t *netif)
{
    tp_assert(netif->ops == &etherif_netif_ops);
    return (etherif_t *)netif;
}

void etherif_destruct(etherif_t *ethif)
{
    netif_unregister(&ethif->netif);
}

void etherif_construct(etherif_t *ethif, const etherif_ops_t *ops)
{
    ethif->ops = ops;
    ethif->ipv4_info = NULL;
    ethif->dhcpc = NULL;
    ethif->udp = NULL;

    ethernet_attach_etherif(ethif);
    netif_register(&ethif->netif, &etherif_netif_ops);
}
