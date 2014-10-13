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
#include "net/net_utils.h"

u16 net_csum(u16 *addr, u16 byte_len)
{
    u32 sum = 0;

    for (; byte_len > 1; byte_len -= 2)
        sum += *addr++;

    if (byte_len)
        sum += *(u8 *)addr;
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    return (u16)~sum;
}

u32 ip_addr_parse(char *buf, int len)
{
    u32 ret = 0;
    u8 *ptr = (u8 *)&ret;

    while (len)
    {
        char c = *buf;

        buf++;
        len--;
        if (c == '.')
        {
            if (ptr - (u8 *)&ret == 3)
                goto Exit;

            ptr++;
        }
        else if (c >= '0' && c <= '9')
            *ptr = *ptr * 10 + c - '0';
        else
            break;
    }

Exit:
    if (ptr - (u8 *)&ret != 3)
        return 0;

    return ret;
}

