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

/* ENC28J60 Control Registers */
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
#define EHT0 0x00
#define EHT1 0x01
#define EHT2 0x02
#define EHT3 0x03
#define EHT4 0x04
#define EHT5 0x05
#define EHT6 0x06
#define EHT7 0x07
#define EPMM0 0x08
#define EPMM1 0x09
#define EPMM2 0x0a
#define EPMM3 0x0b
#define EPMM4 0x0c
#define EPMM5 0x0d
#define EPMM6 0x0e
#define EPMM7 0x0f
#define EPMCSL 0x10
#define EPMCSH 0x11
#define EPMOL 0x14
#define EPMOH 0x15
#define ERXFCON 0x18
#define EPKTCNT 0x19

/* Bank 2 */
#define MACON1 0x00
#define MACON3 0x02
#define MACON4 0x03
#define MABBIPG 0x04
#define MAIPGL 0x06
#define MAIPGH 0x07
#define MACLCON1 0x08
#define MACLCON2 0x09
#define MAMXFLL 0x0a
#define MAMXFLH 0x0b
#define MICMD 0x12
#define MIREGADR 0x14
#define MIWRL 0x16
#define MIWRH 0x17
#define MIRDL 0x18
#define MIRDH 0x19

/* Bank 3 */
#define MAADR5 0x00
#define MAADR6 0x01
#define MAADR3 0x02
#define MAADR4 0x03
#define MAADR1 0x04
#define MAADR2 0x05
#define EBSTSD 0x06
#define EBSTCON 0x07
#define EBSTCSL 0x08
#define EBSTCSH 0x09
#define MISTAT 0x0a
#define EREVID 0x12
#define ECOCON 0x15
#define EFLOCON 0x17
#define EPAUSL 0x18
#define EPAUSH 0x18

/* Common Control Registers (same on all banks) */
#define EIE 0x1b
#define EIR 0x1c
#define ESTAT 0x1d
#define ECON2 0x1e
#define ECON1 0x1f

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
#define VRPS (1<<3)
#define PWRSV (1<<5)
#define PKTDEC (1<<6)
#define AUTOINC (1<<7)

/* ECON1 bits */
#define BSEL0 (1<<0)
#define BSEL1 (1<<1)
#define RXEN (1<<2)
#define TXRTS (1<<3)
#define CSUMEN (1<<4)
#define DMAST (1<<5)
#define RXRST (1<<6)
#define TXRST (1<<7)

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
#define MIIRD (1<<0)
#define MIISCAN (1<<1)

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
#define BUSY (1<<0)
#define SCAN (1<<1)
#define NVALID (1<<2)

/* ECOCON bits */
#define COCON0 (1<<0)
#define COCON1 (1<<1)
#define COCON2 (1<<2)

/* EFLOCON bits */
#define FCEN0 (1<<0)
#define FCEN1 (1<<1)
#define FULDPXS (1<<2)
