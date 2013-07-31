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
#ifndef __DRIVERS_BLOCK_H__
#define __DRIVERS_BLOCK_H__

#define BLOCK_IOCTL_POWER 0
#define BLOCK_IOCTL_SYNC 1
#define BLOCK_IOCTL_GET_SECTOR_SIZE 2
#define BLOCK_IOCTL_GET_SECTOR_COUNT 3
#define BLOCK_IOCTL_GET_BLOCK_SIZE 4
#define BLOCK_IOCTL_ERASE_SECTOR 5

#define BLOCK_DISK_STATUS_NO_INIT 0x01
#define BLOCK_DISK_STATUS_NO_DISK 0x02
#define BLOCK_DISK_STATUS_PROTECTED 0x04

#ifdef CONFIG_MMC
#include "drivers/mmc/mmc.h"
#endif
#include "platform/platform.h"

static inline int block_init(void)
{
#ifdef CONFIG_MMC
    return mmc_spi_disk_init();
#endif
#ifdef CONFIG_PLAT_HAS_BLK
    return platform.block.init();
#endif
    return -1;
}

static inline int block_status(void)
{
#ifdef CONFIG_MMC
    return mmc_spi_disk_status();
#endif
#ifdef CONFIG_PLAT_HAS_BLK
    return platform.block.status();
#endif
    return -1;
}

static inline int block_ioctl(int cmd, void *buf)
{
#ifdef CONFIG_MMC
    return mmc_spi_disk_ioctl(cmd, buf);
#endif
#ifdef CONFIG_PLAT_HAS_BLK
    return platform.block.ioctl(cmd, buf);
#endif
    return -1;
}

static inline int block_read(unsigned char *buf, int sector, int count)
{
#ifdef CONFIG_MMC
    return mmc_spi_disk_read(buf, sector, count);
#endif
#ifdef CONFIG_PLAT_HAS_BLK
    return platform.block.read(buf, sector, count);
#endif
    return -1;
}

static inline int block_write(const unsigned char *buf, int sector, int count)
{
#ifdef CONFIG_MMC
    return mmc_spi_disk_write(buf, sector, count);
#endif
#ifdef CONFIG_PLAT_HAS_BLK
    return platform.block.write(buf, sector, count);
#endif
    return -1;
}

#endif
