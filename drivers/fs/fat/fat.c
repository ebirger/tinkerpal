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
#include "util/debug.h"
#include "util/tstr.h"
#include "mem/tmalloc.h"
#include "drivers/block/block.h"
#include "drivers/fs/vfs.h"
#include "drivers/fs/fat/FatFS/diskio.h"
#include "drivers/fs/fat/FatFS/ff.h"
#include <string.h> /* memcpy */

static FATFS g_fatfs;

DSTATUS disk_initialize(BYTE pdrv)
{
    return block_init() ? STA_NOINIT : 0;
}

DSTATUS disk_status(BYTE pdrv)
{
    return block_status() ? STA_NOINIT : 0;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, BYTE count)
{
    return block_read(buff, sector, count) ? RES_ERROR : RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, BYTE count)
{
    return block_write(buff, sector, count) ? RES_ERROR : RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    switch (cmd)
    {
    case CTRL_SYNC:
	block_ioctl(BLOCK_IOCTL_SYNC, NULL);
	break;
    case GET_SECTOR_SIZE:
	block_ioctl(BLOCK_IOCTL_GET_SECTOR_SIZE, buff);
	break;
    case GET_SECTOR_COUNT:
	block_ioctl(BLOCK_IOCTL_GET_SECTOR_COUNT, buff);
	break;
    case GET_BLOCK_SIZE:
	*(DWORD *)buff = 1;
	break;
    case CTRL_ERASE_SECTOR:
	break;
    }
    return RES_OK;
}

DWORD get_fattime(void)
{
    return 0;
}

static int fat_file_read(tstr_t *content, tstr_t *file_name)
{
    FIL fp = {};
    FILINFO info;
    UINT br;
    char *file_n = NULL;
    int rc = -1;

    file_n = tstr_to_strz(file_name);

    if (f_stat(file_n, &info) != FR_OK)
    {
	/* Silently fail, this may have been a sweep in search of the file */
	goto Exit;
    }

    if (f_open(&fp, file_n, FA_READ|FA_OPEN_EXISTING) != FR_OK)
    {
	tp_err(("file_read: could not open %S\n", file_name));
	goto Exit;
    }

    tstr_alloc(content, info.fsize);
    rc = f_read(&fp, TPTR(content), content->len, &br);
    if (rc != FR_OK || br != content->len)
    {
	tp_err(("Read %d/%d from file %S rc %d\n", br, content->len, 
	    file_name, rc));
	tstr_free(content);
	goto Exit;
    }

    rc = 0;

Exit:
    f_close(&fp);
    tfree(file_n);
    return rc;
}

static int fat_file_write(tstr_t *content, tstr_t *file_name)
{
    FIL fp = {};
    UINT bw;
    char *file_n = NULL;
    int rc = -1;

    file_n = tstr_to_strz(file_name);

    if (f_open(&fp, file_n, FA_WRITE|FA_CREATE_ALWAYS) != FR_OK)
    {
	tp_err(("file_write: could not open %S\n", file_name));
	goto Exit;
    }

    rc = f_write(&fp, TPTR(content), content->len, &bw);
    if (rc != FR_OK || bw != content->len)
    {
	tp_err(("Wrote %d/%d to file %S rc %d\n", bw, content->len, 
	    file_name, rc));
	goto Exit;
    }

    rc = 0;

Exit:
    f_close(&fp);
    tfree(file_n);
    return rc;
}

int fat_readdir(tstr_t *path, readdir_cb_t cb, void *ctx)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    char *cpath;
    int root = 0;

    if (vfs_is_root_path(path))
    {
	root = 1;
	cpath = "/";
    }
    else
	cpath = tstr_to_strz(path);
    res = f_opendir(&dir, cpath);
    if (!root)
	tfree(cpath);

    if (res != FR_OK) 
	return -1;

    for (;;) 
    {
	tstr_t fn = {};
	int rc;

	res = f_readdir(&dir, &fno);
	if (res != FR_OK || fno.fname[0] == '\0') 
	    break;

	/* Ignore dot entry */
	if (fno.fname[0] == '.') 
	    continue;

	tstr_cpy_str(&fn, fno.fname);
	rc = cb(&fn, ctx);
	tstr_free(&fn);
	if (rc)
	    break;
    }

    return res == FR_OK ? 0 : -1;
}

static void fat_init(void)
{
    f_mount(0, &g_fatfs);
}

static void fat_uninit(void)
{
    f_mount(0, 0);
}

const fs_t fat_fs = {
    .name = S("FAT"),
    .init = fat_init,
    .uninit = fat_uninit,
    .file_read = fat_file_read,
    .file_write = fat_file_write,
    .readdir = fat_readdir,
};
