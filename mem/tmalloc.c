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
#include "mem/tmalloc.h"
#include "util/debug.h"

static mem_squeezer_t *squeezers;

#ifdef CONFIG_DLMALLOC

#include "mem/dlmalloc.h"

#define tmalloc_real dlmalloc
#define tfree_real dlfree

#elif defined(CONFIG_MALLOC)

#include <stdlib.h>

#define tmalloc_real malloc
#define tfree_real free

#else

#error malloc type must be defined!

#endif

#ifdef CONFIG_MEM_PROFILING

static int allocated_size = 0;

#ifdef CONFIG_OBJ_REGISTRY

typedef struct obj_reg_rec_t {
    struct obj_reg_rec_t *next;
    void *p;
    char *type;
} obj_reg_rec_t;

static obj_reg_rec_t *obj_registry;
static int obj_registry_max_allocated_size = 0;

static void obj_registry_add_record(void *p, char *type)
{
    obj_reg_rec_t *or = tmalloc_real(sizeof(*or));
    or->p = p;
    or->type = type;
    or->next = obj_registry;
    obj_registry = or;
    if (allocated_size > obj_registry_max_allocated_size)
        obj_registry_max_allocated_size = allocated_size;
}

static void obj_registry_del_record(void *p)
{
    obj_reg_rec_t **iter, *tmp;

    for (iter = &obj_registry; *iter && (*iter)->p != p; iter = &(*iter)->next);
    if (!(tmp = *iter))
    {
        /* XXX: error */
        return;
    }

    *iter = (*iter)->next;
    tfree_real(tmp);
}

static void obj_registry_stats(void)
{
    obj_reg_rec_t *rec;

    tp_out("Max allocated size %db = %dKb\n", 
        obj_registry_max_allocated_size, 
        obj_registry_max_allocated_size >> 10);
    tp_out("Current registry objects:\n");

    for (rec = obj_registry; rec; rec = rec->next)
    {
        int sz = *(((unsigned long *)rec->p) - 1);

        tp_out("%p size %d type %s\n", rec->p, sz, rec->type);
    }
}

static void obj_registry_uninit(void)
{
    obj_reg_rec_t *rec;

    while ((rec = obj_registry))
    {
        obj_registry = obj_registry->next;
        tfree_real(rec);
    }
}

#endif

void tmalloc_stats(void)
{
    tp_out("Total allocated size %db = %dKb\n", allocated_size, 
        allocated_size >> 10);
#ifdef CONFIG_DLMALLOC_STATISTICS
    dlmalloc_stats();
#endif
#ifdef CONFIG_OBJ_REGISTRY
    obj_registry_stats();
#endif
}

void tmalloc_uninit(void)
{
    tmalloc_stats();

#ifdef CONFIG_OBJ_REGISTRY
    obj_registry_uninit();
#endif
}

static inline void *_tmalloc(int sz, char *type)
{
    unsigned long *p;

    sz += sizeof(unsigned long);
    allocated_size += sz;

#ifdef CONFIG_MEM_ALLOCATION_LIMIT
    if (allocated_size > CONFIG_MEM_ALLOCATION_LIMIT_BYTES)
    {
        tp_err("Reached maximal allowed allocation limit:\n"
            "Currently allocated: %d\n"
            "Limit: %d\n", allocated_size,
            CONFIG_MEM_ALLOCATION_LIMIT_BYTES);
        goto Error;
    }
#endif
    
    p = tmalloc_real(sz);
    if (!p)
        goto Error;

    *p++ = sz;

#ifdef CONFIG_OBJ_REGISTRY
    obj_registry_add_record(p, type);
#endif
    return p;

Error:
    tmalloc_uninit();
    return NULL;
}

void tfree(void *data)
{
    unsigned long *p = data;

    if (!p)
        return;

#ifdef CONFIG_OBJ_REGISTRY
    obj_registry_del_record(p);
#endif

    p--;
    allocated_size -= *p;
    tfree_real(p);
}

#else

static inline void *_tmalloc(int sz, char *type)
{
    void *p;

    if (!(p = tmalloc_real(sz)))
        return NULL;

    tp_debug("Allocated %p %d %s\n", p, sz, type);
    return p;
}

void tfree(void *data)
{
    if (!data)
        return;

    tp_debug("freeing %p\n", data);
    tfree_real(data);
}

void tmalloc_stats(void)
{
}

void tmalloc_uninit(void)
{
}

#endif

void *tmalloc(int sz, char *type)
{
    void *p;
    int need_squeeze;
    mem_squeezer_t *s;

    if ((p = _tmalloc(sz, type)))
        return p;

    need_squeeze = sz;
    tp_debug("Squeezing, need %d\n", sz);
    for (s = squeezers; s; s = s->next)
    {
        need_squeeze -= s->squeeze(s, need_squeeze >= 0 ? need_squeeze : 0);
        tp_debug("Squeezed %d so far\n", sz - need_squeeze);
    }

    if (need_squeeze > 0 || !(p = _tmalloc(sz, type)))
        tp_crit("allocation error\n");
    
    return p;
}

void tmalloc_register_squeezer(mem_squeezer_t *squeezer)
{
    squeezer->next = squeezers;
    squeezers = squeezer;
}

void tmalloc_unregister_squeezer(mem_squeezer_t *squeezer)
{
    mem_squeezer_t **s;

    for (s = &squeezers; *s != squeezer; s = &(*s)->next);
    (*s) = squeezer->next;
    squeezer->next = NULL;
}

void tmalloc_init(void)
{
}
