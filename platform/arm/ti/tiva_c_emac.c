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
#include "platform/arm/ti/tiva_c_emac.h"
#include "util/tp_types.h"
#include "util/tp_misc.h"
#include "util/debug.h"
#include "util/event.h"
#include "mem/tmalloc.h"
#include "net/etherif.h"
#include <driverlib/rom_map.h>
#include <driverlib/emac.h>
#include <driverlib/sysctl.h>
#include <driverlib/interrupt.h>
#include <inc/hw_memmap.h>
#include <inc/hw_emac.h>
#include <inc/hw_ints.h>

typedef struct {
    etherif_t ethif;
    u32 istat;
} tiva_c_emac_t;

static u8 test_mac[] = { 0, 1, 2, 3, 4, 5 };
static tiva_c_emac_t g_eth; /* Singleton */

static int tiva_c_emac_link_status(etherif_t *ethif)
{
    return MAP_EMACPHYRead(EMAC0_BASE, 0, EPHY_BMSR) & EPHY_BMSR_LINKSTAT ? 1 :
        0;
}

static void tiva_c_emac_mac_addr_get(etherif_t *ethif, eth_mac_t *mac)
{
    MAP_EMACAddrGet(EMAC0_BASE, 0, mac->mac);
}

static int tiva_c_emac_packet_recv(etherif_t *ethif, u8 *buf, int size)
{
    return 0;
}

static void tiva_c_emac_packet_xmit(etherif_t *ethif, u8 *buf, int size)
{
}

static void tiva_c_emac_free(etherif_t *ethif)
{
    etherif_destruct(ethif);
}

static const etherif_ops_t tiva_c_emac_etherif_ops = {
    .link_status = tiva_c_emac_link_status,
    .mac_addr_get = tiva_c_emac_mac_addr_get,
    .packet_recv = tiva_c_emac_packet_recv,
    .packet_xmit = tiva_c_emac_packet_xmit,
    .free = tiva_c_emac_free,
};

static void phy_info(void)
{
    u32 id1, id2;
    
    id1 = MAP_EMACPHYRead(EMAC0_BASE, 0, EPHY_ID1);
    id2 = MAP_EMACPHYRead(EMAC0_BASE, 0, EPHY_ID2);
    tp_out("PHY OUI %x:%x\n", (id1 & EPHY_ID1_OUIMSB_M) >> EPHY_ID1_OUIMSB_S,
        (id2 & EPHY_ID2_OUILSB_M) >> EPHY_ID2_OUILSB_S);
    tp_out("Model Number %x\n",
        (id2 & EPHY_ID2_VNDRMDL_M) >> EPHY_ID2_VNDRMDL_S);
    tp_out("Revision Number %x\n", 
        (id2 & EPHY_ID2_MDLREV_M) >> EPHY_ID2_MDLREV_S);
}

static void phy_cfg(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);

    /* Clear any pending PHY interrupts */
    MAP_EMACPHYRead(EMAC0_BASE, 0, EPHY_MISR1);
    MAP_EMACPHYRead(EMAC0_BASE, 0, EPHY_MISR2);

    /* Enable link change interrupt */
    MAP_EMACPHYWrite(EMAC0_BASE, 0, EPHY_MISR1, EPHY_MISR1_LINKSTATEN);

    /* Start an auto-negotiation cycle. */
    MAP_EMACPHYWrite(EMAC0_BASE, 0, EPHY_BMCR, EPHY_BMCR_ANEN | 
        EPHY_BMCR_RESTARTAN);
}

int tiva_c_emac_event_process(void)
{
    MAP_IntDisable(INT_EMAC0);
    if (!g_eth.istat)
    {
        MAP_IntEnable(INT_EMAC0);
        return 0;
    }

    if (g_eth.istat & EMAC_INT_PHY)
    {
        /* Ack PHY interrupt */
        MAP_EMACPHYRead(EMAC0_BASE, 0, EPHY_MISR1);
        etherif_port_changed(&g_eth.ethif);
        MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_PHY);
    }

    g_eth.istat = 0;

    MAP_IntEnable(INT_EMAC0);
    return 0;
}

void tiva_c_emac_isr(void)
{
    u32 istat;

    istat = MAP_EMACIntStatus(EMAC0_BASE, true);

    /* Mask interrupts until they are handled */
    MAP_EMACIntDisable(EMAC0_BASE, istat);

    /* Clear interrupt status */
    MAP_EMACIntClear(EMAC0_BASE, istat);

    tp_out("Tiva C EMAC ISR %x\n", istat);

    g_eth.istat |= istat;
}

static void hw_init(void)
{
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
    MAP_SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);

    MAP_EMACAddrSet(EMAC0_BASE, 0, test_mac);

    /* Enable the Ethernet Controller transmitter and receiver */
    MAP_EMACTxEnable(EMAC0_BASE);
    MAP_EMACRxEnable(EMAC0_BASE);

    /* Clear pending interrupts */
    MAP_EMACIntClear(EMAC0_BASE, MAP_EMACIntStatus(EMAC0_BASE, false));

    /* Enable Ethernet Interrupts */
    MAP_EMACIntEnable(EMAC0_BASE, EMAC_INT_PHY);
    
    /* Enable the Ethernet Interrupt handler */
    MAP_IntEnable(INT_EMAC0);
}

netif_t *tiva_c_emac_new(void)
{
    g_eth.istat = 0;
    etherif_construct(&g_eth.ethif, &tiva_c_emac_etherif_ops);

    hw_init();
    phy_cfg();
    phy_info();

    return &g_eth.ethif.netif;
}
