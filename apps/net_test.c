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
#include "net/ether.h"
#include "platform/platform_consts.h"
#include "apps/cli.h"
#if defined(CONFIG_LINUX_PACKET_ETH)
#include "drivers/net/linux_packet_eth.h"
#elif defined(CONFIG_STELLARIS_ETH)
#include "platform/arm/stellaris/stellaris_eth.h"
#elif defined(CONFIG_ENC28J60)
#include "drivers/net/enc28j60.h"
#else
#error No Network device available
#endif

static etherif_t *ethif;

void net_test_quit(void)
{
    ethernet_detach_etherif(ethif);
    etherif_free(ethif);
}

void net_test_process_line(tstr_t *line)
{
    console_printf("Ok\n");
}

static cli_client_t net_test_cli_client = {
    .process_line = net_test_process_line,
    .quit = net_test_quit,
};

void app_start(int argc, char *argv[])
{
    tp_out(("TinkerPal Application - Net Test\n"));

#if defined(CONFIG_LINUX_PACKET_ETH)
    if (argc != 2)
	tp_crit(("Usage: %s <network interface>\n", argv[0]));

    ethif = linux_packet_eth_new(argv[1]);
#elif defined(CONFIG_STELLARIS_ETH)
    ethif = stellaris_eth_new();
#elif defined(CONFIG_ENC28J60)
    ethif = enc28j60_new(RES(SPI_RESOURCE_ID_BASE, 1, 0),
	RES(GPIO_RESOURCE_ID_BASE, PE3, 0),
	RES(GPIO_RESOURCE_ID_BASE, PF4, 0));
#endif

    tp_assert(ethif);

    ethernet_attach_etherif(ethif);

    cli_start(&net_test_cli_client);
}
