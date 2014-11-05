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
#include "platform/arm/cortex-m.h"

static volatile uint32_t ticks;
static uint32_t last_ticks, cm_time_sec, cm_time_msec;

#ifdef CONFIG_GCC
static char *heap_end = 0;
extern unsigned long _heap_bottom;
extern unsigned long _heap_top;
extern unsigned long _stack_top;

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

void cortex_m_fault_isr(void) __attribute__((naked));
void cortex_m_fault_isr(void)
{
    __asm volatile
    (
        " mrs r0, msp\n"
        " ldr r2, handler2_address_const\n"
        " bx r2\n"
        " handler2_address_const: .word cortex_m_oops\n"
    );
}

void cortex_m_oops(unsigned char *fault_sp)
{
    struct regs {
        unsigned int r0;
        unsigned int r1;
        unsigned int r2;
        unsigned int r3;
        unsigned int r12;
        unsigned int lr;
        unsigned int pc;
        unsigned int psr;
    } *regs = (struct regs *)fault_sp;
    unsigned char *real_sp = (unsigned char *)(regs + 1);

    tp_out(("Cortex-M Hard Fault\n"));
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define P(reg) tp_out(("%s = 0x%08x\n", #reg, regs->reg))
    P(r0);
    P(r1);
    P(r2);
    P(r3);
    P(r12);
    P(lr);
    P(pc);
    P(psr);
    tp_out(("Stack:\n"));
    hexdump(real_sp, MIN(256, (int)((unsigned char *)&_stack_top - real_sp)));
    while(1);
}
#else
void cortex_m_fault_isr(void)
{
    while(1);
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
    ticks++;
}

void cortex_m_get_time_from_boot(uint32_t *sec, uint32_t *usec)
{
    uint32_t cur_ticks = ticks;

    cm_time_msec += cur_ticks - last_ticks;
    last_ticks = cur_ticks;

    while (cm_time_msec >= 1000)
    {
	cm_time_msec -= 1000;
	cm_time_sec++;
    }
    *sec = cm_time_sec;
    *usec = cm_time_msec * 1000;
}

void cortex_m_panic(void)
{
    while (1);
}
