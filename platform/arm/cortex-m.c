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

static volatile unsigned int cm_time_sec = 0;
static volatile unsigned int cm_time_usec = 0;

#ifdef CONFIG_GCC
static char *heap_end = 0;
extern unsigned long _heap_bottom;
extern unsigned long _heap_top;

void *_sbrk(unsigned int incr)
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
#endif

void cortex_m_meminfo(void)
{
#ifdef CONFIG_GCC
    tp_out(("Heap: Total %d Allocated %d Remaining %d\n", 
        (&_heap_top - &_heap_bottom) * 4,
        ((unsigned long *)heap_end - &_heap_bottom) * 4,
        (&_heap_top - (unsigned long *)heap_end) * 4));
#endif
}

void cortex_m_reset_isr(void)
{
#if defined(CONFIG_GCC)
    extern unsigned long _etext;
    extern unsigned long _data;
    extern unsigned long _edata;
    extern unsigned long _bss;
    extern unsigned long _ebss;
    unsigned long *src, *dst;

    /* Copy the data segment initializers from flash to RAM */
    src = &_etext;
    dst = &_data;
    while (dst < &_edata)
        *dst++ = *src++;

    /* Zero out the bss segment */
    src = &_bss;
    while (src < &_ebss)
        *src++ = 0;

#elif defined(CONFIG_TI_CCS5)
    /* Jump to the CCS C initialization routine.  This will enable the
     * floating-point unit as well, so that does not need to be done here.
     */
    __asm("    .global _c_int00\n"
          "    b.w     _c_int00");
#endif
}

void cortex_m_systick_isr(void)
{
    cm_time_usec += 1000;
    if (cm_time_usec == 1000000)
    {
	cm_time_usec = 0;
	cm_time_sec++;
    }
}

void cortex_m_get_time_from_boot(unsigned int *sec, unsigned int *usec)
{
    *sec = cm_time_sec;
    *usec = cm_time_usec;
}

void cortex_m_panic(void)
{
    while (1);
}
