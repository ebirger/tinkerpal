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
#define _DARWIN_C_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h> 
#include <errno.h> 
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "platform/unix/unix.h"
#include "drivers/serial/serial_platform.h"

static struct timeval boot;

int unix_select(int ms, unix_fd_event_map_t *map)
{
    fd_set rfds;
    struct timeval tv;
    int rc, sec, max_fd = 0;
    unix_fd_event_map_t *iter;

    FD_ZERO(&rfds);

    for (iter = map; iter->event != -1; iter++)
    {
        FD_SET(iter->in_fd, &rfds);
        if (iter->in_fd > max_fd - 1)
            max_fd = iter->in_fd + 1;
    }

    sec = ms / 1000;
    tv.tv_sec = sec;
    tv.tv_usec = (ms - sec * 1000) * 1000;

    rc = select(max_fd, &rfds, NULL, NULL, ms ? &tv : NULL);
    if (rc == -1)
        perror("select");
    if (rc > 0)
    {
        for (iter = map; iter->event != -1; iter++)
        {
            if (!FD_ISSET(iter->in_fd, &rfds))
                continue;
                
            serial_event_trigger(iter->event);
        }
    }
    return rc;
}

static int get_event_fd(int event, unix_fd_event_map_t *map, int in)
{
    unix_fd_event_map_t *iter;

    for (iter = map; iter->event != -1 && iter->event != event; iter++);
    if (iter->event == -1)
    {
        printf("invalid event id %d\n", event);
        exit(1);
    }

    return in ? iter->in_fd : iter->out_fd;
}

int unix_read(int event, char *buf, int size, unix_fd_event_map_t *map)
{
    int rsize, fd = get_event_fd(event, map, 1);

    rsize = read(fd, buf, size);
    if (!rsize)
    {
        printf("fd %d closed\n", fd);
        exit(1);
    }
    if (rsize < 0)
    {
        perror("read");
        exit(1);
    }
    return rsize;
}

int unix_write(int event, char *buf, int size, unix_fd_event_map_t *map)
{
    int fd = get_event_fd(event, map, 0);

    if (write(fd, buf, size) != size)
    {
        perror("write");
        exit(1);
    }
    return size;
}

void unix_panic(void)
{
    exit(1);
}

int unix_get_ticks_from_boot(void)
{
    struct timeval now, diff;

    gettimeofday(&now, NULL);

    timersub(&now, &boot, &diff);
    return diff.tv_sec * 1000 + diff.tv_usec / 1000;
}

void unix_get_time_from_boot(unsigned int *sec, unsigned int *usec)
{
    struct timeval now, diff;

    gettimeofday(&now, NULL);

    timersub(&now, &boot, &diff);
    *sec = diff.tv_sec;
    *usec = diff.tv_usec;
}

void unix_set_term_raw(int fd, int raw)
{
    struct termios term;

    tcgetattr(fd, &term);
    if (raw)
        term.c_lflag &= ~(ECHO|ICANON);
    else
        term.c_lflag |= (ECHO|ICANON);
    tcsetattr(fd, TCSANOW, &term);
}

void unix_set_nonblock(int fd)
{
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}

void unix_init(void)
{
    gettimeofday(&boot, NULL);
}

void unix_uninit(void)
{
}
