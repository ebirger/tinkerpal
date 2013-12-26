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
#include "drivers/net/enc28j60.h"
#include "util/tp_types.h"
#include "util/tp_misc.h"
#include "util/debug.h"
#include "util/event.h"
#include "mem/tmalloc.h"
#include "drivers/spi/spi.h"
#include "drivers/gpio/gpio.h"

/* ENC28J60 Control Registers */
/* Use bits on the register addresses for meta data */
#define REG_MII_MAC (1<<7)
#define REG_ADDR_MASK 0x1f
#define BANK1 (1<<5)
#define BANK2 (2<<5)
#define BANK3 (3<<5)
#define REG_BANK(addr) (((addr) >> 5) & 0x3)
#define REG(addr) ((addr) & REG_ADDR_MASK)

/* Bank 0 */
#define ERDPTL 0x00
#define ERDPTH 0x01
#define EWRPTL 0x02
#define EWRPTH 0x03
#define ETXSTL 0x04
#define ETXSTH 0x05
#define ETXNDL 0x06
#define ETXNDH 0x07
#define ERXSTL 0x08
#define ERXSTH 0x09
#define ERXNDL 0x0a
#define ERXNDH 0x0b
#define ERXRDPTL 0x0c
#define ERXRDPTH 0x0d
#define ERXWRPTL 0x0e
#define ERXWRPTH 0x0f
#define EDMASTL 0x10
#define EDMASTH 0x11
#define EDMANDL 0x12
#define EDMANDH 0x13
#define EDMADSTL 0x14
#define EDMADSTH 0x15
#define EDMACSL 0x16
#define EDMACSH 0x17

/* Bank 1 */
#define EHT0 (0x00 | BANK1)
#define EHT1 (0x01 | BANK1)
#define EHT2 (0x02 | BANK1)
#define EHT3 (0x03 | BANK1)
#define EHT4 (0x04 | BANK1)
#define EHT5 (0x05 | BANK1)
#define EHT6 (0x06 | BANK1)
#define EHT7 (0x07 | BANK1)
#define EPMM0 (0x08 | BANK1)
#define EPMM1 (0x09 | BANK1)
#define EPMM2 (0x0a | BANK1)
#define EPMM3 (0x0b | BANK1)
#define EPMM4 (0x0c | BANK1)
#define EPMM5 (0x0d | BANK1)
#define EPMM6 (0x0e | BANK1)
#define EPMM7 (0x0f | BANK1)
#define EPMCSL (0x10 | BANK1)
#define EPMCSH (0x11 | BANK1)
#define EPMOL (0x14 | BANK1)
#define EPMOH (0x15 | BANK1)
#define ERXFCON (0x18 | BANK1)
#define EPKTCNT (0x19 | BANK1)

/* Bank 2 */
#define MACON1 (0x00 | REG_MII_MAC | BANK2)
#define MACON2 (0x01 | REG_MII_MAC | BANK2)
#define MACON3 (0x02 | REG_MII_MAC | BANK2)
#define MACON4 (0x03 | REG_MII_MAC | BANK2)
#define MABBIPG (0x04 | REG_MII_MAC | BANK2)
#define MAIPGL (0x06 | REG_MII_MAC | BANK2)
#define MAIPGH (0x07 | REG_MII_MAC | BANK2)
#define MACLCON1 (0x08 | REG_MII_MAC | BANK2)
#define MACLCON2 (0x09 | REG_MII_MAC | BANK2)
#define MAMXFLL (0x0a | REG_MII_MAC | BANK2)
#define MAMXFLH (0x0b | REG_MII_MAC | BANK2)
#define MICMD (0x12 | REG_MII_MAC | BANK2)
#define MIREGADR (0x14 | REG_MII_MAC | BANK2)
#define MIWRL (0x16 | REG_MII_MAC | BANK2)
#define MIWRH (0x17 | REG_MII_MAC | BANK2)
#define MIRDL (0x18 | REG_MII_MAC | BANK2)
#define MIRDH (0x19 | REG_MII_MAC | BANK2)

/* Bank 3 */
#define MAADR1 (0x00 | REG_MII_MAC | BANK3)
#define MAADR0 (0x01 | REG_MII_MAC | BANK3)
#define MAADR3 (0x02 | REG_MII_MAC | BANK3)
#define MAADR2 (0x03 | REG_MII_MAC | BANK3)
#define MAADR5 (0x04 | REG_MII_MAC | BANK3)
#define MAADR4 (0x05 | REG_MII_MAC | BANK3)
#define EBSTSD (0x06 | BANK3)
#define EBSTCON (0x07 | BANK3)
#define EBSTCSL (0x08 | BANK3)
#define EBSTCSH (0x09 | BANK3)
#define MISTAT (0x0a | REG_MII_MAC)
#define EREVID (0x12 | BANK3)
#define ECOCON (0x15 | BANK3)
#define EFLOCON (0x17 | BANK3)
#define EPAUSL (0x18 | BANK3)
#define EPAUSH (0x19 | BANK3)

/* Common Control Registers (same on all banks) */
#define EIE 0x1b
#define EIR 0x1c
#define ESTAT 0x1d
#define ECON2 0x1e
#define ECON1 0x1f

/* PHY Registers */
#define PHCON1 0x00
#define PHSTAT1 0x01
#define PHID1 0x02
#define PHID2 0x03
#define PHCON2 0x10
#define PHSTAT2 0x11
#define PHIE 0x12
#define PHIR 0x13
#define PHLCON 0x14

/* EIE bits */
#define RXERIE (1<<0)
#define TXERIE (1<<1)
#define TXIE (1<<3)
#define LINKIE (1<<4)
#define DMAIE (1<<5)
#define PKTIE (1<<6)
#define INTIE (1<<7)

/* EIR bits */
#define RXERIF (1<<0)
#define TXERIF (1<<1)
#define TXIF (1<<3)
#define LINKIF (1<<4)
#define DMAIF (1<<5)
#define PKTIF (1<<6)

/* ESTAT bits */
#define CLKRDY (1<<0)
#define TXABRT (1<<1)
#define RXBUSY (1<<2)
#define LATECOL (1<<4)
#define BUFER (1<<6)
#define INT (1<<7)

/* ECON2 bits */
#define VRPS (1<<3) /* Voltage Regulator Power Save Enable */
#define PWRSV (1<<5) /* Power Save Enable */
#define PKTDEC (1<<6) /* Packet Decrement */
#define AUTOINC (1<<7) /* Automatic Buffer Pointer Increment Enable */

/* ECON1 bits */
#define BSEL0 (1<<0) /* Bank Select 0 */
#define BSEL1 (1<<1) /* Bank Select 1 */
#define RXEN (1<<2) /* Receive Enable */
#define TXRTS (1<<3) /* Transmit Request to Send */
#define CSUMEN (1<<4) /* DMA Checksum Enable */
#define DMAST (1<<5) /* DMS Start and Busy Status */
#define RXRST (1<<6) /* Receive Logic Reset */
#define TXRST (1<<7) /* Transmit Logic Reset */

/* ERXFCON bits */
#define BCEN (1<<0)
#define MCEN (1<<1)
#define HTEN (1<<2)
#define MPEN (1<<3)
#define PMEN (1<<4)
#define CRCEN (1<<5)
#define ANDOR (1<<6)
#define UCEN (1<<7)

/* MACON1 bits */
#define MARXEN (1<<0)
#define PASSALL (1<<1)
#define RXPAUS (1<<2)
#define TXPAUS (1<<3)
#define LOOPBK (1<<4)

/* MACON2 bits */
#define TFUNRST (1<<0)
#define MAYXRST (1<<1)
#define RFUNRST (1<<2)
#define MARXRST (1<<3)
#define RNDRST (1<<6)
#define MARST (1<<7)

/* MACON3 bits */
#define FULDPX (1<<0)
#define FRMLNEN (1<<1)
#define HFRMEN (1<<2)
#define PHDREN (1<<3)
#define TXCRCEN (1<<4)
#define PADCFG0 (1<<5)
#define PADCFG1 (1<<6)
#define PADCFG2 (1<<7)

/* MACON4 bits */
#define NOBKOFF (1<<4)
#define BPEN (1<<5)
#define DEFER (1<<6)

/* MICMD bits */
#define MIIRD (1<<0) /* MII Read Enable */
#define MIISCAN (1<<1) /* MII Scan Enable */

/* EBSTCON bits */
#define BISTST (1<<0)
#define TME (1<<1)
#define TMSEL0 (1<<2)
#define TMSEL1 (1<<3)
#define PSEL (1<<4)
#define PSV0 (1<<5)
#define PSV1 (1<<6)
#define PSV2 (1<<7)

/* MISTAT bits */
#define BUSY (1<<0) /* MII Management Busy */
#define SCAN (1<<1) /* MII Management Scan Operation */
#define NVALID (1<<2) /* MII Management Read Data Not Valid */

/* ECOCON bits */
#define COCON0 (1<<0)
#define COCON1 (1<<1)
#define COCON2 (1<<2)

/* EFLOCON bits */
#define FCEN0 (1<<0)
#define FCEN1 (1<<1)
#define FULDPXS (1<<2)

/* PHCON1 bits */
#define PDPXMD (1<<8)
#define PPWRSV (1<<11)
#define PLOOPBK (1<<14)
#define PRST (1<<15)

/* PHSTAT1 bits */
#define JBSTAT (1<<1) /* PHY Latching Jabber Status */
#define LLSTAT (1<<2) /* PHY Latching Link Status */
#define PHDPX (1<<11) /* PHY Half-Duplex Capable */
#define PFDPX (1<<12) /* PHY Full-Duplex Capable */

/* PHCON2 bits */
#define HDLDIS (1<<8)
#define JABBER (1<<10)
#define TXDIS (1<<13)
#define FRCLNK (1<<14)

/* PHSTAT2 bits */
#define PLRITY (1<<5) /* Polarity Status */
#define DPXSTAT (1<<9) /* PHY Duplex Status */
#define LSTAT (1<<10) /* PHY Link Status */
#define COLSTAT (1<<11) /* PHY Collision Status */
#define RXSTAT (1<<12) /* PHY Receive Status */
#define TXSTAT (1<<13) /* PHY Transmit Status */

/* PHIE bits */
#define PGEIE (1<<1)
#define PLNKIE (1<<4)

/* PHIR bits */
#define PGIF (1<<2)
#define PLNKIF (1<<4)

/* PHLCON bits */
#define STRCH (1<<1)
#define LFRQ0 (1<<2)
#define LFRQ1 (1<<3)
#define LBCFG0 (1<<4)
#define LBCFG1 (1<<5)
#define LBCFG2 (1<<6)
#define LBCFG3 (1<<7)
#define LACFG0 (1<<8)
#define LACFG1 (1<<9)
#define LACFG2 (1<<10)
#define LACFG3 (1<<11)

/* ENC28J60 SPI Opcodes */
#define ENC28J60_OPCODE_RCR 0x00 /* Read Control Register */
#define ENC28J60_OPCODE_RBM 0x3a /* Read Buffer Memory */
#define ENC28J60_OPCODE_WCR 0x40 /* Write Control Register */
#define ENC28J60_OPCODE_WBM 0x7a /* Write Buffer Memory */
#define ENC28J60_OPCODE_BFS 0x80 /* Bit Field Set */
#define ENC28J60_OPCODE_BFC 0xa0 /* Bit Field Clear */
#define ENC28J60_OPCODE_SRC 0xff /* System Reset Command */

#define RX_BUF_START 0
#define RX_BUF_END (0x1fff - 1536) /* One packet for transmission */
#define TX_BUF_START (RX_BUF_END + 1)

#define RX_STAT_OK (1 << 7) /* bit 23 - 16 bits of packet length */

typedef struct {
    etherif_t ethif;
    event_t irq_event;
    int irq_event_id;
    int spi_port;
    int cs;
    int intr;
    u8 bank;
    u16 next_pkt_ptr;
} enc28j60_t;

#define ETHIF_TO_ENC28J60(x) container_of(x, enc28j60_t, ethif)

static u8 test_mac[] = { 0, 1, 2, 3, 4, 5 };

static inline void cs_low(enc28j60_t *e)
{
    /* asserts the CS pin to the card */
    gpio_digital_write(e->cs, 0);
}

static inline void cs_high(enc28j60_t *e)
{
    /* de-asserts the CS pin to the card */
    gpio_digital_write(e->cs, 1);
}

static u8 read_op(enc28j60_t *e, u8 op, u8 addr)
{
    u8 ret;

    cs_low(e);
    spi_send(e->spi_port, op | REG(addr));
    if (addr & REG_MII_MAC)
    {
	/* MAC and MII registers also return a dummy byte */
	spi_receive(e->spi_port);
    }
    ret = (u8)spi_receive(e->spi_port);
    cs_high(e);
    return ret;
}

static void write_op(enc28j60_t *e, u8 op, u8 addr, u8 data)
{
    cs_low(e);
    spi_send(e->spi_port, op | REG(addr));
    spi_send(e->spi_port, data);
    cs_high(e);
}

static void bank_select(enc28j60_t *e, u8 addr)
{
    u8 bank = REG_BANK(addr), reg = REG(addr);

    if (e->bank == bank)
	return;

    if (reg >= EIE && reg <= ECON1)
    {
	/* No need to switch banks, all banks have these registers */
	return;
    }

    write_op(e, ENC28J60_OPCODE_BFC, ECON1, BSEL0 | BSEL1);
    write_op(e, ENC28J60_OPCODE_BFS, ECON1, bank);

    e->bank = bank;
}

static inline u8 ctrl_reg_read(enc28j60_t *e, u8 reg)
{
    bank_select(e, reg);
    return read_op(e, ENC28J60_OPCODE_RCR, reg);
}

static inline void ctrl_reg_write(enc28j60_t *e, u8 reg, u8 data)
{
    bank_select(e, reg);
    write_op(e, ENC28J60_OPCODE_WCR, reg, data);
}

static inline void ctrl_wreg_write(enc28j60_t *e, u8 reg, u16 data)
{
    bank_select(e, reg);
    write_op(e, ENC28J60_OPCODE_WCR, reg, data & 0xff);
    write_op(e, ENC28J60_OPCODE_WCR, reg + 1, data >> 8);
}

static inline void ctrl_reg_bits_clear(enc28j60_t *e, u8 reg, u8 mask)
{
    bank_select(e, reg);
    write_op(e, ENC28J60_OPCODE_BFC, reg, mask);
}

static inline void ctrl_reg_bits_set(enc28j60_t *e, u8 reg, u8 mask)
{
    bank_select(e, reg);
    write_op(e, ENC28J60_OPCODE_BFS, reg, mask);
}

static u16 phy_reg_read(enc28j60_t *e, u8 phy_reg)
{
    u16 ret;

    ctrl_reg_write(e, MIREGADR, phy_reg);
    ctrl_reg_bits_set(e, MICMD, MIIRD);
    while (ctrl_reg_read(e, MISTAT) & BUSY);
    ctrl_reg_bits_clear(e, MICMD, MIIRD);
    ret = ctrl_reg_read(e, MIRDL);
    ret |= ctrl_reg_read(e, MIRDH) << 8;
    return ret;
}

static void phy_reg_write(enc28j60_t *e, u8 phy_reg, u16 data)
{
    ctrl_reg_write(e, MIREGADR, phy_reg);
    ctrl_reg_write(e, MIWRL, data & 0xff);
    ctrl_reg_write(e, MIWRH, (data >> 8) & 0xff);
    while (ctrl_reg_read(e, MISTAT) & BUSY);
}

static int enc28j60_link_status(etherif_t *ethif)
{
    return phy_reg_read(ETHIF_TO_ENC28J60(ethif), PHSTAT2) & LSTAT ? 1 : 0;
}

static inline u32 ticks(void)
{
    return (u32)platform.get_ticks_from_boot();
}

static int chip_reset(enc28j60_t *e)
{
    u32 start = ticks();
    int ready;

    tp_info(("Resetting ENC28J60\n"));

    write_op(e, ENC28J60_OPCODE_SRC, 0, 0);

    /* According to ENC28J60 Silicon Errata for B7, we need to wait at least
     * 1 ms after reset since the CLKRDY bit may not be cleared in time, and
     * the declared timeout is not guarenteed
     */
    platform.msleep(3);

    /* Wait for reset to complete */
    while (!(ready = (ctrl_reg_read(e, ESTAT) & CLKRDY)) && 
	(ticks() - start < 1000));

    return ready ? 0 : -1;
}

static void erxrdpt_set(enc28j60_t *e, u16 addr)
{
    /* Per B7 Silicon Errata, ERXRDPT must not be set to an even address */
    addr--;

    if (addr < RX_BUF_START || addr > RX_BUF_END)
	addr = RX_BUF_END;

    ctrl_wreg_write(e, ERXRDPTL, addr);
}

static void rx_buf_init(enc28j60_t *e, u16 start, u16 end)
{
    e->next_pkt_ptr = start;
    ctrl_wreg_write(e, ERXSTL, start);
    ctrl_wreg_write(e, ERXNDL, end);
    erxrdpt_set(e, start);
}

static void mac_addr_conf(enc28j60_t *e, u8 mac[6])
{
    ctrl_reg_write(e, MAADR0, mac[5]);
    ctrl_reg_write(e, MAADR1, mac[4]);
    ctrl_reg_write(e, MAADR2, mac[3]);
    ctrl_reg_write(e, MAADR3, mac[2]);
    ctrl_reg_write(e, MAADR4, mac[1]);
    ctrl_reg_write(e, MAADR5, mac[0]);
}

static void enc28j60_mac_addr_get(etherif_t *ethif, eth_mac_t *mac)
{
    enc28j60_t *e = ETHIF_TO_ENC28J60(ethif);

    mac->mac[5] = ctrl_reg_read(e, MAADR0);
    mac->mac[4] = ctrl_reg_read(e, MAADR1);
    mac->mac[3] = ctrl_reg_read(e, MAADR2);
    mac->mac[2] = ctrl_reg_read(e, MAADR3);
    mac->mac[1] = ctrl_reg_read(e, MAADR4);
    mac->mac[0] = ctrl_reg_read(e, MAADR5);
}

static void chip_init(enc28j60_t *e)
{
    tp_out(("ENC28J60 Init\n"));
    if (chip_reset(e))
    {
	tp_err(("ENC28J60 Reset failed. Is it connected?\n"));
	return;
    }

    tp_out(("Ethernet Rev ID: %d\n", ctrl_reg_read(e, EREVID) & 0x1f));
    tp_out(("PHY ID %x:%x\n", phy_reg_read(e, PHID1), phy_reg_read(e, PHID2)));

    /* Enable auto increment of the ERDPT/EWRPT pointers */
    ctrl_reg_bits_set(e, ECON2, AUTOINC);
    rx_buf_init(e, RX_BUF_START, RX_BUF_END);

    /* MAC config */
    /* Allow only targeted unicast or broadcast packets with valid CRCs */
    ctrl_reg_write(e, ERXFCON, UCEN | CRCEN | BCEN);
    ctrl_reg_bits_clear(e, MACON2, MARST);
    ctrl_reg_bits_set(e, MACON1, MARXEN);
    /* Pad short frames to 60 bytes, add CRC. No Full-Duplex */
    ctrl_reg_write(e, MACON3, PADCFG0 | TXCRCEN);
    ctrl_wreg_write(e, MAMXFLL, 1518);
    /* Back-to-Back Inter-Packet Gap - Half-Duplex */
    ctrl_reg_write(e, MABBIPG, 0x12);
    /* Non Back-to-Back Inter-Packet Gap - Half Duplex */
    ctrl_wreg_write(e, MAIPGL, 0x0c12);
    mac_addr_conf(e, test_mac);
    /* Enable packet reception */
    ctrl_reg_bits_set(e, ECON1, RXEN);

    /* PHY config */
    phy_reg_write(e, PHCON1, 0); /* Normal operation - Half-Duplex */
    phy_reg_write(e, PHCON2, HDLDIS); /* Do not loop back xmitted packets */

    /* Enable interrupts */
    ctrl_reg_bits_set(e, EIE, LINKIE | INTIE | PKTIE | TXIE);
    /* Enable PHY interrupts */
    phy_reg_write(e, PHIE, PGEIE | PLNKIE);
}

static void packet_complete(enc28j60_t *e)
{
    /* Free packet memory by advancing ERXRDPT */
    erxrdpt_set(e, e->next_pkt_ptr);
    /* Indicate packet processing is complete */
    ctrl_reg_bits_set(e, ECON2, PKTDEC);
    ctrl_reg_bits_set(e, EIE, PKTIE); /* Unmask packet received interrupt */
}

static int enc28j60_packet_recv(etherif_t *ethif, u8 *buf, int size)
{
    enc28j60_t *e = ETHIF_TO_ENC28J60(ethif);
    u8 header[6];
    u16 stat, packet_length;

    ctrl_wreg_write(e, ERDPTL, e->next_pkt_ptr);

    cs_low(e);

    spi_send(e->spi_port, ENC28J60_OPCODE_RBM);
    spi_receive_mult(e->spi_port, header, 6);

#define MK_U16(a, b) ((((u16)a) << 8) | b)
    e->next_pkt_ptr = MK_U16(header[1], header[0]);
    packet_length = MK_U16(header[3], header[2]);
    stat = MK_U16(header[5], header[4]);
    if (!(stat & RX_STAT_OK))
    {
	tp_info(("Invalid packet received\n"));
	goto Exit;
    }

    if (size > packet_length)
	size = packet_length;
    
    spi_receive_mult(e->spi_port, buf, size);

Exit:
    cs_high(e);
    packet_complete(e);
    return size;
}

static void enc28j60_packet_xmit(etherif_t *ethif, u8 *buf, int size)
{
    enc28j60_t *e = ETHIF_TO_ENC28J60(ethif);
    u8 ctrl = 0;

    /* Set buffer start */
    ctrl_wreg_write(e, EWRPTL, TX_BUF_START);
    ctrl_wreg_write(e, ETXSTL, TX_BUF_START);
    cs_low(e);
    spi_send(e->spi_port, ENC28J60_OPCODE_WBM);
    /* Write ctrl byte + data */
    spi_send(e->spi_port, ctrl);
    spi_send_mult(e->spi_port, buf, size);
    cs_high(e);
    /* Set buffer end */
    ctrl_wreg_write(e, ETXNDL, TX_BUF_START + size + 1);
    /* Start xmit */
    ctrl_reg_bits_set(e, ECON1, TXRTS);
}

static void packet_received(enc28j60_t *e)
{
    tp_info(("ENC28J60 packet received\n"));

    etherif_packet_received(&e->ethif);
}

static void packet_xmitted(enc28j60_t *e)
{
    tp_err(("ENC28J60 packet transmitted\n"));

    /* TODO: read packet xmit status */

    etherif_packet_xmitted(&e->ethif);
}

static void link_status_changed(enc28j60_t *e)
{
    tp_info(("ENC28J60 Link state change - state %d\n", 
	enc28j60_link_status(e)));
    etherif_port_changed(&e->ethif);
}

static void enc28j60_isr(event_t *ev, u32 resource_id)
{
    enc28j60_t *e = container_of(ev, enc28j60_t, irq_event);
    u8 eir;

    eir = ctrl_reg_read(e, EIR);
    tp_debug(("ENC28J60 ISR %x\n", eir));

    if (eir & LINKIF)
    {
	phy_reg_read(e, PHIR); /* Ack PHY interrupt */
	ctrl_reg_bits_clear(e, EIR, LINKIF); /* Ack interrupt */
	link_status_changed(e);
    }
    if (eir & PKTIF)
    {
	ctrl_reg_bits_clear(e, EIR, PKTIF); /* Ack interrupt */
	ctrl_reg_bits_clear(e, EIE, PKTIE); /* Mask packet received interrupt */
	packet_received(e);
    }

    if (eir & TXIF)
    {
	ctrl_reg_bits_clear(e, EIR, TXIF); /* Ack interrupt */
	packet_xmitted(e);
    }
}

static void enc28j60_free(etherif_t *ethif)
{
    enc28j60_t *e = ETHIF_TO_ENC28J60(ethif);

    event_watch_del(e->irq_event_id);
    etherif_destruct(ethif);
    tfree(e);
}

static const etherif_ops_t enc28j60_etherif_ops = {
    .link_status = enc28j60_link_status,
    .mac_addr_get = enc28j60_mac_addr_get,
    .packet_recv = enc28j60_packet_recv,
    .packet_xmit = enc28j60_packet_xmit,
    .free = enc28j60_free,
};

etherif_t *enc28j60_new(int spi_port, int cs, int intr)
{
    enc28j60_t *e = tmalloc_type(enc28j60_t);

    e->spi_port = spi_port;
    e->cs = cs;
    e->intr = intr;
    e->irq_event.trigger = enc28j60_isr;
    etherif_construct(&e->ethif, &enc28j60_etherif_ops);

    spi_init(spi_port);
    spi_set_max_speed(spi_port, 8000000);

    gpio_set_pin_mode(cs, GPIO_PM_OUTPUT);
    gpio_set_pin_mode(intr, GPIO_PM_INPUT_PULLUP);
    cs_high(e);

    e->irq_event_id = event_watch_set(intr, &e->irq_event);

    chip_init(e);
    return &e->ethif;
}
