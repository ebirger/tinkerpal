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
#include "platform/arm/ti/stellaris_eth.h"
#include "util/tp_types.h"
#include "util/tp_misc.h"
#include "util/debug.h"
#include "util/event.h"
#include "mem/tmalloc.h"
#include "net/etherif.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_ethernet.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/ethernet.h"
#include "driverlib/interrupt.h"

typedef struct {
    etherif_t ethif;
    u32 istat;
} stellaris_eth_t;

static u8 test_mac[] = { 0, 1, 2, 3, 4, 5 };
static stellaris_eth_t g_eth; /* Singleton */

static int stellaris_eth_link_status(etherif_t *ethif)
{
    return MAP_EthernetPHYRead(ETH_BASE, PHY_MR1) & PHY_MR1_LINK ? 1 : 0;
}

static void stellaris_eth_mac_addr_get(etherif_t *ethif, eth_mac_t *mac)
{
    MAP_EthernetMACAddrGet(ETH_BASE, mac->mac);
}

static int stellaris_eth_packet_recv(etherif_t *ethif, u8 *buf, int size)
{
    long length;

    length = MAP_EthernetPacketGetNonBlocking(ETH_BASE, buf, size);
    MAP_EthernetIntEnable(ETH_BASE, ETH_INT_RX);
    if (length < 0)
        tp_err(("Buffer too small (%d). Packet length %d\n", size, -length));
    return (int)length;
}

static void stellaris_eth_packet_xmit(etherif_t *ethif, u8 *buf, int size)
{
    long length;

    length = MAP_EthernetPacketPut(ETH_BASE, buf, size);
    if (length < 0)
        tp_err(("No space for packet (%d). Space left %d\n", size, -length));
}

static void stellaris_eth_free(etherif_t *ethif)
{
    etherif_destruct(ethif);
}

static const etherif_ops_t stellaris_eth_etherif_ops = {
    .link_status = stellaris_eth_link_status,
    .mac_addr_get = stellaris_eth_mac_addr_get,
    .packet_recv = stellaris_eth_packet_recv,
    .packet_xmit = stellaris_eth_packet_xmit,
    .free = stellaris_eth_free,
};

static void phy_info(void)
{
    u32 mr2, mr3;
    
    mr2 = MAP_EthernetPHYRead(ETH_BASE, PHY_MR2);
    mr3 = MAP_EthernetPHYRead(ETH_BASE, PHY_MR3);
    tp_out(("PHY OUI %x:%x\n", (mr2 & PHY_MR2_OUI_M) >> PHY_MR2_OUI_S,
        (mr3 & PHY_MR3_OUI_M) >> PHY_MR3_OUI_S));
    tp_out(("Model Number %x\n", (mr3 & PHY_MR3_MN_M) >> PHY_MR3_MN_S));
    tp_out(("Revision Number %x\n", (mr3 & PHY_MR3_RN_M) >> PHY_MR3_RN_S));
}

static void phy_cfg(void)
{
    /* Enable link change interrupt */
    MAP_EthernetPHYWrite(ETH_BASE, PHY_MR17,
        MAP_EthernetPHYRead(ETH_BASE, PHY_MR17) | PHY_MR17_LSCHG_IE);
}

int stellaris_eth_event_process(void)
{
    MAP_IntDisable(INT_ETH);
    if (!g_eth.istat)
    {
        MAP_IntEnable(INT_ETH);
        return 0;
    }

    if (g_eth.istat & ETH_INT_PHY)
    {
        /* Ack PHY interrupt */
        MAP_EthernetPHYWrite(ETH_BASE, PHY_MR17,
            MAP_EthernetPHYRead(ETH_BASE, PHY_MR17) | PHY_MR17_LSCHG_INT);

        etherif_port_changed(&g_eth.ethif);
        MAP_EthernetIntEnable(ETH_BASE, ETH_INT_PHY);
    }
    
    if (g_eth.istat & ETH_INT_RX)
    {
        etherif_packet_received(&g_eth.ethif);
        /* Interrupt will be unmasked after read */
    }
    
    if (g_eth.istat & ETH_INT_TX)
    {
        etherif_packet_xmitted(&g_eth.ethif);
        MAP_EthernetIntEnable(ETH_BASE, ETH_INT_TX);
    }

    g_eth.istat = 0;

    MAP_IntEnable(INT_ETH);
    return 1;
}

void stellaris_ethernet_isr(void)
{
    u32 istat;

    istat = MAP_EthernetIntStatus(ETH_BASE, true);

    /* Mask interrupts until they are handled */
    MAP_EthernetIntDisable(ETH_BASE, istat);

    /* Clear interrupt status */
    MAP_EthernetIntClear(ETH_BASE, istat);

    tp_debug(("Stellaris Ethernet ISR %x\n", istat));

    g_eth.istat |= istat;
}

static void hw_init(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

    /* Disable all Ethernet Interrupts. */
    MAP_EthernetIntDisable(ETH_BASE, ETH_INT_PHY | ETH_INT_MDIO |
        ETH_INT_RXER | ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX);
    /* Disable any possible pending interrupts */
    MAP_EthernetIntClear(ETH_BASE, MAP_EthernetIntStatus(ETH_BASE, false));

    /* Initialize the Ethernet Controller. */
    MAP_EthernetInitExpClk(ETH_BASE, platform.get_system_clock());

    /* Configure the Ethernet Controller */
    MAP_EthernetConfigSet(ETH_BASE, ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN |
        ETH_CFG_TX_PADEN | ETH_CFG_RX_AMULEN);
    
    MAP_EthernetMACAddrSet(ETH_BASE, test_mac);

    /* Enable the Ethernet Controller transmitter and receiver */
    MAP_EthernetEnable(ETH_BASE);

    /* Enable the Ethernet Interrupt handler */
    MAP_IntEnable(INT_ETH);

    /* Enable Ethernet Interrupts */
    MAP_EthernetIntEnable(ETH_BASE, ETH_INT_PHY | ETH_INT_RX | ETH_INT_TX);
}

netif_t *stellaris_eth_new(void)
{
    g_eth.istat = 0;
    etherif_construct(&g_eth.ethif, &stellaris_eth_etherif_ops);

    hw_init();
    phy_cfg();
    phy_info();

    return &g_eth.ethif.netif;
}
