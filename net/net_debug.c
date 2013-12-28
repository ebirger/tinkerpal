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
#include "util/tprintf.h"
#include "net/net_debug.h"

static char buf[18];

char *eth_mac_serialize(eth_mac_t *m)
{
    tsnprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", m->mac[0],
	m->mac[1], m->mac[2], m->mac[3], m->mac[4], m->mac[5]);
    return buf;
}

#define D(field, fmt, args...) tp_out((field ": " fmt "\n", ##args))

void eth_hdr_dump(eth_hdr_t *hdr)
{
    D("DST", "%s", eth_mac_serialize(&hdr->dst));
    D("SRC", "%s", eth_mac_serialize(&hdr->src));
    D("ETHERTYPE", "%04x", ntohs(hdr->eth_type));
}

static char *ip_addr_serialize(u8 ip[])
{
    tsnprintf(buf, sizeof(buf), "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    return buf;
}

void arp_packet_dump(arp_packet_t *arp)
{
    D("HTYPE", "%4x", ntohs(arp->htype));
    D("PTYPE", "%4x", ntohs(arp->ptype));
    D("HLEN", "%2d", arp->hlen);
    D("PLEN", "%2d", arp->plen);
    D("OPER", "%4x", ntohs(arp->oper));
    D("SHA", "%s", eth_mac_serialize(&arp->sha));
    D("SPA", "%s", ip_addr_serialize(arp->spa));
    D("THA", "%s", eth_mac_serialize(&arp->tha));
    D("TPA", "%s", ip_addr_serialize(arp->tpa));
}

void ip_hdr_dump(ip_hdr_t *iph)
{
    D("Version", "%d", iph->ver);
    D("IHL", "%d", iph->ihl);
    D("DSCP", "%x", iph->dscp);
    D("Total Length", "%d", ntohs(iph->tot_len));
    D("ID", "%x", ntohs(iph->id));
    D("Frag Offset", "%x", ntohs(iph->frag_off));
    D("TTL", "%d", iph->ttl);
    D("Protocol", "%x", iph->protocol);
    D("Checksum", "%x", ntohs(iph->checksum));
    D("SRC", "%s", ip_addr_serialize(iph->src_addr));
    D("DST", "%s", ip_addr_serialize(iph->dst_addr));
}

void udp_hdr_dump(udp_hdr_t *udph)
{
    D("SPORT", "%d", ntohs(udph->src_port));
    D("DPORT", "%d", ntohs(udph->dst_port));
    D("Length", "%d", ntohs(udph->length));
    D("Checksum", "%x", ntohs(udph->checksum));
}

void dhcp_msg_dump(dhcp_msg_t *msg)
{
    D("OP", "%x", msg->op);
    D("HTYPE", "%x", msg->htype);
    D("HLEN", "%x", msg->hlen);
    D("HOPS", "%x", msg->hops);
    D("XID", "%x", htonl(msg->xid));
    D("SECS", "%x", htons(msg->secs));
    D("FLAGS", "%x", htons(msg->flags));
    D("CIADDR", "%s", ip_addr_serialize((u8 *)&msg->ciaddr));
    D("YIADDR", "%s", ip_addr_serialize((u8 *)&msg->yiaddr));
    D("SIADDR", "%s", ip_addr_serialize((u8 *)&msg->siaddr));
    D("GIADDR", "%s", ip_addr_serialize((u8 *)&msg->giaddr));
    D("CHADDR", "%s", eth_mac_serialize((eth_mac_t *)&msg->chaddr));
}
