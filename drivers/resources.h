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
#ifndef __RESOURCES_H__
#define __RESOURCES_H__

#include "util/tp_types.h"

#ifndef CONFIG_16_BIT

/* Resource structure in 32 bits:
 * minor: bits [0-7]
 * major: bits [8-23]
 * base: bits [24-31]
 */
typedef u32 resource_t;

#define RES_MIN_SHIFT 0
#define RES_MIN_MASK 0xff
#define RES_MAJ_SHIFT 8
#define RES_MAJ_MASK 0xffff
#define RES_BASE_SHIFT 24
#define RES_BASE_MASK 0xff

#else

/* Resource structure in 16 bits:
 * minor: bits [0-2]
 * major: bits [3-9]
 * base: bits [10-15]
 */
typedef u16 resource_t;

#define RES_MIN_SHIFT 0
#define RES_MIN_MASK 0x7
#define RES_MAJ_SHIFT 3
#define RES_MAJ_MASK 0x7f
#define RES_BASE_SHIFT 10
#define RES_BASE_MASK 0x3f

#endif

#define RES(base, maj, min) \
    ((resource_t)((((base) & RES_BASE_MASK) << RES_BASE_SHIFT) | \
	(((maj) & RES_MAJ_MASK) << RES_MAJ_SHIFT) | \
	(((min) & RES_MIN_MASK) << RES_MIN_SHIFT)))
#define RES_MIN(res) (((res) >> RES_MIN_SHIFT) & RES_MIN_MASK)
#define RES_MAJ(res) (((res) >> RES_MAJ_SHIFT) & RES_MAJ_MASK)
#define RES_BASE(res) (((res) >> RES_BASE_SHIFT) & RES_BASE_MASK)

#define GPIO_RESOURCE_ID_BASE 0x01
#define UART_RESOURCE_ID_BASE 0x02
#define SPI_RESOURCE_ID_BASE 0x03
#define ETHERIF_RESOURCE_ID_BASE 0x04

#endif
