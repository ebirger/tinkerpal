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
#define _XOPEN_SOURCE 600 
#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include "util/debug.h"
#include "drivers/block/block.h"
#include "platform/platform.h"
#include "platform/unix/unix.h"
#include "main/tp.h"

static int pty_fd = -1;
static int ext_tty_fd = -1;

static int unix_sim_event_fd_count;
static unix_fd_event_map_t unix_sim_event_fd_map[NUM_IDS + 1] = { 
    [0 ... NUM_IDS] = { .event = -1 }
};

#define STDIN_FD 0
#define STDOUT_FD 1

FILE *block_disk;
#define SEC_SIZE 512

void unix_sim_add_fd_event_to_map(int event, int in_fd,
    int out_fd)
{
    unix_sim_event_fd_map[unix_sim_event_fd_count].event = event;
    unix_sim_event_fd_map[unix_sim_event_fd_count].in_fd = in_fd;
    unix_sim_event_fd_map[unix_sim_event_fd_count].out_fd = out_fd;
    unix_sim_event_fd_count++;
}

void unix_sim_remove_fd_event_from_map(int event)
{
    unix_fd_event_map_t *from, *to;

    for (to = unix_sim_event_fd_map; to->event != event; to++);
    tp_assert(to->event != -1);
    from = to;
    while (to->event != -1)
        *to++ = *++from;
    unix_sim_event_fd_count--;
}

#ifdef CONFIG_PLATFORM_EMULATION_PTY_TERM
static int pty_open(void)
{
    int fdm;

    fdm = posix_openpt(O_RDWR);
    if (fdm < 0)
    { 
        perror("posix_openpt()\n");
        return -1; 
    }

    if (grantpt(fdm))
    {
        perror("grantpt()\n");
        return -1;
    }

    if (unlockpt(fdm))
    {
        perror("unlockpt()\n");
        return -1;
    }

    printf("PTY at : %s\n", ptsname(fdm));

    return fdm;
}
#endif

#ifdef CONFIG_PLATFORM_EMULATION_EXT_TTY
static int ext_tty_open(void)
{
    struct termios options;
    int fd;

    if ((fd = open(CONFIG_PLATFORM_EMULATION_EXT_TTY_PATH,
        O_RDWR | O_NONBLOCK)) < 0)
    {
        return -1;
    }

    tcgetattr(fd, &options);
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    tcsetattr(fd, TCSANOW, &options);

    return fd;
}
#endif

static int sim_unix_select(int ms)
{
    return unix_select(ms, unix_sim_event_fd_map);
}

static int sim_unix_serial_read(int id, char *buf, int size)
{
    return unix_read(id, buf, size, unix_sim_event_fd_map);
}

static int sim_unix_serial_write(int id, char *buf, int size)
{
    return unix_write(id, buf, size, unix_sim_event_fd_map);
}

static int sim_unix_serial_enable(int id, int enabled)
{
    int in_fd, out_fd;

    if (!enabled)
        return -1; /* Not implemented yet */

    switch (id)
    {
    case STDIO_ID:
        in_fd = STDIN_FD;
        out_fd = STDOUT_FD;
        break;
#ifdef CONFIG_PLATFORM_EMULATION_PTY_TERM
    case PTY_ID:
        in_fd = out_fd = pty_fd = pty_open();
        break;
#endif
#ifdef CONFIG_PLATFORM_EMULATION_EXT_TTY
    case EXT_TTY_ID:
        in_fd = out_fd = ext_tty_fd = ext_tty_open();
        break;
#endif
    default:
        tp_err("Unsupported Serial ID %d\n", id);
        return -1;
    }

    unix_set_nonblock(in_fd);
    unix_set_term_raw(in_fd, 1);
    unix_sim_add_fd_event_to_map(id, in_fd, out_fd);
    return 0;
}

#ifdef CONFIG_PLATFORM_EMULATION_BLOCK_DISK
int sim_unix_block_disk_init(void)
{
    block_disk = fopen(CONFIG_PLATFORM_EMULATION_BLOCK_DISK_PATH, "r+");
    if (!block_disk)
    {
        tp_err("Could not open block disk file\n");
        return -1;
    }

    return 0;
}

static int sim_unix_block_disk_ioctl(int cmd, void *buf)
{
    switch (cmd)
    {
    case BLOCK_IOCTL_SYNC:
        fflush(block_disk);
        return 0;
    case BLOCK_IOCTL_GET_SECTOR_SIZE:
        *(int *)buf = SEC_SIZE;
        return 0;
    case BLOCK_IOCTL_GET_SECTOR_COUNT:
        *(int *)buf = CONFIG_PLATFORM_EMULATION_BLOCK_DISK_NUM_SECTORS;
        return 0;
    case BLOCK_IOCTL_GET_BLOCK_SIZE:
    case BLOCK_IOCTL_ERASE_SECTOR:
        break;
    }
    return -1;
}

int sim_unix_block_disk_status(void)
{
    return block_disk ? 0 : -1;
}

int sim_unix_block_disk_read(unsigned char *buf, int sector, int count)
{
    int n;

    if (fseek(block_disk, sector * SEC_SIZE, SEEK_SET) < 0)
    {
        tp_err("failed seek @ %d\n", sector * SEC_SIZE);
        return -1;
    }

    n = fread(buf, count, SEC_SIZE, block_disk);
    if (n != count * SEC_SIZE)
    {
        tp_err("read %d/%d bytes\n", n, count * SEC_SIZE);
        return -1;
    }
    tp_info("sector %d count %d read %d bytes\n", sector, count, n);
    return 0;
}

int sim_unix_block_disk_write(const unsigned char *buf, int sector, int count)
{
    int n;

    if (fseek(block_disk, sector * SEC_SIZE, SEEK_SET))
    {
        tp_err("failed seek @ %d\n", sector * SEC_SIZE);
        return -1;
    }

    n = fwrite(buf, count, SEC_SIZE, block_disk);
    if (n != count * SEC_SIZE)
    {
        tp_err("wrote %d/%d bytes\n", n, count * SEC_SIZE);
        return -1;
    }
    tp_info("sector %d count %d wrote %d bytes\n", sector, count, n);
    return 0;
}
#endif

void sigint_handler(int s)
{
    serial_event_signal(STDIO_ID);
}

static void set_sigint_handler(void (*cb)(int))
{
    struct sigaction handler = {
	.sa_handler = sigint_handler,
    };

    sigemptyset(&handler.sa_mask);
    sigaction(SIGINT, &handler, NULL);
}

static void sim_unix_uninit(void)
{
    printf("Unix Platform Simulator Uninit\n");
    if (pty_fd != -1)
    {
        unix_set_term_raw(pty_fd, 0);
        close(pty_fd);
    }
    if (ext_tty_fd != -1)
    {
        unix_set_term_raw(ext_tty_fd, 0);
        close(ext_tty_fd);
    }
    if (block_disk)
        fclose(block_disk);
    set_sigint_handler(SIG_DFL);
    unix_set_term_raw(STDIN_FD, 0);
    unix_uninit();
}

static void sim_unix_init(void)
{
    printf("Unix Platform Simulator Init\n");

    unix_init();

    set_sigint_handler(sigint_handler);

    atexit(sim_unix_uninit);
}

const platform_t platform = {
    .serial = {
        .enable = sim_unix_serial_enable,
        .read = sim_unix_serial_read,
        .write = sim_unix_serial_write,
    },
#ifdef CONFIG_PLATFORM_EMULATION_BLOCK_DISK
    .block = {
        .init = sim_unix_block_disk_init,
        .status = sim_unix_block_disk_status,
        .ioctl = sim_unix_block_disk_ioctl,
        .read = sim_unix_block_disk_read,
        .write = sim_unix_block_disk_write,
    },
#endif
    .init = sim_unix_init,
    .select = sim_unix_select,
    .get_time_from_boot = unix_get_time_from_boot,
    .panic = unix_panic,
};

int main(int argc, char *argv[])
{
    return tp_main(argc, argv);
}
