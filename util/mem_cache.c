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
#include <stdio.h> // NULL
#include "util/mem_cache.h"
#include "util/tmalloc.h"
#include "util/tp_types.h"
#include "util/debug.h"

#define NUM_ITEMS 10

struct mem_cache_t {
    mem_squeezer_t squeezer; /* must be first */
    mem_cache_t *next;
    int item_size;
    char *free_list;
};

static void mem_cache_uninit(mem_cache_t *cache)
{
    mem_cache_t *tmp;

    while ((tmp = cache))
    {
	cache = cache->next;
	tfree(tmp);
    }
}

static mem_cache_t *mem_cache_init(int item_size)
{
    mem_cache_t *cache = tmalloc(sizeof(mem_cache_t)+(item_size * NUM_ITEMS), 
	"mem cache");
    char *item;
    int i;

    tp_debug(("Created mem cache %p, item size %d\n", cache, item_size));
    cache->next = NULL;
    cache->squeezer.squeeze = NULL;
    cache->item_size = item_size;
    cache->free_list = item = (char *)(cache + 1);
    for (i = 0; i < NUM_ITEMS; i++)
    {
	char *next;

	next = i < NUM_ITEMS - 1 ? item + item_size : NULL;
	*((uint_ptr_t *)item) = (uint_ptr_t)next;
	tp_debug(("current item %p, next %p\n", item, next));
	item = next;
    }
    return cache;
}

static inline int mem_cache_is_unused(mem_cache_t *cache)
{
    int num_free;
    uint_ptr_t *next = (uint_ptr_t *)cache->free_list;

    for (num_free = 0; next; next = (uint_ptr_t *)*next, num_free++);
    return num_free == NUM_ITEMS;
}

static int mem_cache_squeeze(mem_squeezer_t *squeezer, int size)
{
    mem_cache_t *cache = (mem_cache_t *)squeezer, *next;
    int freed = 0;

    tp_info(("mem_cache_squeeze: requested to free %d bytes\n", size));
    while ((next = cache->next))
    {
	if (!mem_cache_is_unused(next))
	{
	    cache = next;
	    continue;
	}

	cache->next = next->next;
	next->next = NULL;
	mem_cache_uninit(next);
	freed += sizeof(mem_cache_t) + (next->item_size * NUM_ITEMS);
    }

    tp_info(("mem_cache_squeeze: freed %d bytes\n", freed));
    return freed;
}

mem_cache_t *mem_cache_create(int item_size)
{
    mem_cache_t *cache = mem_cache_init(item_size);

    cache->squeezer.squeeze = mem_cache_squeeze;
    tmalloc_register_squeezer(&cache->squeezer);
    return cache;
}

void mem_cache_destroy(mem_cache_t *cache)
{
    tmalloc_unregister_squeezer(&cache->squeezer);
    mem_cache_uninit(cache);
}

void *mem_cache_alloc(mem_cache_t *cache)
{
    char *item, *next;

    if (!(item = cache->free_list))
    {
	mem_cache_t **iter;

	for (iter = &cache->next; *iter && !(*iter)->free_list; 
	    iter = &(*iter)->next);
	if (*iter && (*iter)->free_list)
	    cache = *iter;
	else
	    cache = *iter = mem_cache_init(cache->item_size);
	item = cache->free_list;
    }

    next = (char *)*((uint_ptr_t *)item);
    cache->free_list = next;
    tp_debug(("Allocated %p\n", item));
    return (void *)item;
}

static inline int is_in_cache(mem_cache_t *cache, void *ptr)
{
    char *item = ptr, *start = (char *)(cache + 1);

    return item >= start && item < (start + cache->item_size * NUM_ITEMS);
}

static inline mem_cache_t *mem_cache_find(mem_cache_t *cache, void *ptr)
{
    for (; cache && !is_in_cache(cache, ptr); cache = cache->next);
    return cache;
}

void mem_cache_free(mem_cache_t *cache, void *ptr)
{
    char *item = ptr;

    cache = mem_cache_find(cache, ptr);
    tp_debug(("freeing %p from cache %p\n", item, cache));
    *((uint_ptr_t *)item) = (uint_ptr_t)cache->free_list;
    cache->free_list = item;
}
