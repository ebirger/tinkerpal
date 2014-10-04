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
#include "util/debug.h"
#include "util/cli.h"
#include "net/net_debug.h"
#include "net/netif.h"
#include "platform/platform_consts.h"
#include "boards/board.h"
#if defined(CONFIG_LINUX_ETH)
#include "platform/unix/linux_eth.h"
#elif defined(CONFIG_STELLARIS_ETH)
#include "platform/arm/ti/stellaris_eth.h"
#elif defined(CONFIG_TIVA_C_EMAC)
#include "platform/arm/ti/tiva_c_emac.h"
#elif defined(CONFIG_ENC28J60)
#include "drivers/net/enc28j60.h"
#else
#error No Network device available
#endif

static netif_t *netif;

static void net_test_quit(void)
{
    netif_ip_disconnect(netif);
    netif_free(netif);
}

void got_ip(event_t *e, u32 resource_id, u64 timestamp)
{
    tp_out(("DHCP: IP address aquired\n"));
    netif_on_event_clear(netif, NETIF_EVENT_IPV4_CONNECTED);
}

static event_t got_ip_event = {
    .trigger = got_ip
};

static void net_test_process_line(tstr_t *line)
{
    if (!tstr_cmp(line, &S("connect")))
    {
        netif_on_event_set(netif, NETIF_EVENT_IPV4_CONNECTED,
            &got_ip_event);
        netif_ip_connect(netif);
    }
    if (!tstr_cmp(line, &S("ip")))
    {
        u32 addr = ntohl(netif_ip_addr_get(netif));

        console_printf("IP address: %s\n", ip_addr_serialize(addr));
    }
    if (!tstr_cmp(line, &S("link")))
        console_printf("Link status: %d\n", netif_link_status(netif));

    console_printf("Ok\n");
}

static cli_client_t net_test_cli_client = {
    .process_line = net_test_process_line,
    .quit = net_test_quit,
};

void app_start(int argc, char *argv[])
{
    eth_mac_t mac;

    tp_out(("TinkerPal Application - Net Test\n"));

#if defined(CONFIG_LINUX_ETH)
    if (argc != 2)
        tp_crit(("Usage: %s <network interface>\n", argv[0]));

    netif = linux_eth_new(argv[1]);
#elif defined(CONFIG_STELLARIS_ETH)
    netif = stellaris_eth_new();
#elif defined(CONFIG_TIVA_C_EMAC)
    netif = tiva_c_emac_new();
#elif defined(CONFIG_ENC28J60)
    netif = enc28j60_new(&board.enc28j60_params);
#endif

    tp_assert(netif);

    netif_mac_addr_get(netif, &mac);
    tp_out(("Interface MAC address: %s\n", eth_mac_serialize(&mac)));
    cli_start(&net_test_cli_client);
}
