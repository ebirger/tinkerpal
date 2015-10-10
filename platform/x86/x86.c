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
#include "platform/x86/vga_term.h"

static char *heap_end = 0;
extern unsigned long _heap_bottom;
extern unsigned long _heap_top;

static int x86_serial_enable(int u, int enabled)
{
    return 0;
}

static int x86_serial_write(int u, char *buf, int size)
{
    while (size--)
        vga_term_putchar(*buf++);
    return 0;
}

static void x86_meminfo(void)
{
    tp_out("Heap: Total %d Allocated %d Remaining %d\n", 
        (&_heap_top - &_heap_bottom) * 4,
        ((unsigned long *)heap_end - &_heap_bottom) * 4,
        (&_heap_top - (unsigned long *)heap_end) * 4);
}

static void x86_init(void)
{
    vga_term_init();
}

void *sbrk(unsigned int incr)
{
    static char *prev_heap_end;

    if (heap_end == 0)
        heap_end = (void *)&_heap_bottom;

    prev_heap_end = heap_end;

    if (heap_end + incr > (char *)&_heap_top) 
        return (void *)0;

    heap_end += incr;

    return (void *)prev_heap_end;
}

const platform_t platform = {
    .serial = {
        .enable = x86_serial_enable,
        .write = x86_serial_write,
    },
    .mem = {
        .info = x86_meminfo,
    },
    .init = x86_init,
};

void kernel_main(int argc, char *argv[])
{
    extern int tp_main(int argc, char *argv[]);

    tp_main(argc, argv);
}
