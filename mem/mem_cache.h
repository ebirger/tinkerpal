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
#ifndef __MEM_CACHE_H__
#define __MEM_CACHE_H__

#define mem_cache_create_type(typ) mem_cache_create(sizeof(typ), #typ "_cache")

#ifdef CONFIG_MEM_CACHE

typedef struct mem_cache_t mem_cache_t;

mem_cache_t *mem_cache_create(int item_size, char *name);
void mem_cache_destroy(mem_cache_t *cache);

void *mem_cache_alloc(mem_cache_t *cache);
void mem_cache_free(mem_cache_t *cache, void *ptr);
void mem_cache_stats(void);

#else

#include "mem/tmalloc.h"
#include "util/tp_types.h"
#include "util/debug.h"

typedef void mem_cache_t;

static inline mem_cache_t *mem_cache_create(int item_size, char *name) 
{ 
    return (mem_cache_t *)(uint_ptr_t)item_size;
}

static inline void mem_cache_destroy(mem_cache_t *cache) { }

static inline void *mem_cache_alloc(mem_cache_t *cache)
{
    return tmalloc((uint_ptr_t)cache, "cache item");
}

static inline void mem_cache_free(mem_cache_t *cache, void *ptr)
{
    tfree(ptr);
}

static inline void mem_cache_stats(void) { }

#endif

#endif
