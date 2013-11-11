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

typedef struct mem_cache_block_t {
    struct mem_cache_block_t *next;
    char *free_list;
} mem_cache_block_t;

struct mem_cache_t {
    mem_squeezer_t squeezer; /* must be first */
    mem_cache_t *next;
    int item_size;
    char *name;
    mem_cache_block_t *head;
};

static mem_cache_t *mem_cache_head;

#define BLOCK_SZ(item_sz) (sizeof(mem_cache_block_t) + (item_sz) * NUM_ITEMS)

static void mem_cache_block_destroy(mem_cache_block_t *block)
{
    mem_cache_block_t *tmp;

    while ((tmp = block))
    {
	block = block->next;
	tfree(tmp);
    }
}

static mem_cache_block_t *mem_cache_block_create(int item_size)
{
    mem_cache_block_t *block = tmalloc(BLOCK_SZ(item_size), "mem cache block");
    char *item;
    int i;

    tp_debug(("Created mem cache block %p, item size %d\n", block, item_size));
    block->next = NULL;
    block->free_list = item = (char *)(block + 1);
    for (i = 0; i < NUM_ITEMS; i++)
    {
	char *next;

	next = i < NUM_ITEMS - 1 ? item + item_size : NULL;
	*((uint_ptr_t *)item) = (uint_ptr_t)next;
	tp_debug(("current item %p, next %p\n", item, next));
	item = next;
    }
    return block;
}

static inline int mem_cache_block_is_unused(mem_cache_block_t *block)
{
    int num_free;
    uint_ptr_t *next = (uint_ptr_t *)block->free_list;

    for (num_free = 0; next; next = (uint_ptr_t *)*next, num_free++);
    return num_free == NUM_ITEMS;
}

static int mem_cache_squeeze(mem_squeezer_t *squeezer, int size)
{
    mem_cache_t *cache = (mem_cache_t *)squeezer;
    mem_cache_block_t *block = cache->head, *next;
    int freed = 0;

    tp_info(("mem_cache_squeeze: requested to free %d bytes\n", size));
    while ((next = block->next))
    {
	if (!mem_cache_block_is_unused(next))
	{
	    block = next;
	    continue;
	}

	block->next = next->next;
	next->next = NULL;
	mem_cache_block_destroy(next);
	freed += BLOCK_SZ(cache->item_size);
    }

    tp_info(("mem_cache_squeeze: freed %d bytes\n", freed));
    return freed;
}

mem_cache_t *mem_cache_create(int item_size, char *name)
{
    mem_cache_t *cache = tmalloc_type(mem_cache_t);

    cache->head = mem_cache_block_create(item_size);
    cache->item_size = item_size;
    cache->squeezer.squeeze = mem_cache_squeeze;
    cache->name = name;
    tmalloc_register_squeezer(&cache->squeezer);
    cache->next = mem_cache_head;
    mem_cache_head = cache;
    return cache;
}

void mem_cache_destroy(mem_cache_t *cache)
{
    mem_cache_t **iter;

    for (iter = &mem_cache_head; *iter != cache; iter = &(*iter)->next);
    *iter = cache->next;
    tmalloc_unregister_squeezer(&cache->squeezer);
    mem_cache_block_destroy(cache->head);
    tfree(cache);
}

void *mem_cache_alloc(mem_cache_t *cache)
{
    char *item, *next;
    mem_cache_block_t *block = cache->head;

    if (!(item = block->free_list))
    {
	mem_cache_block_t **iter;

	for (iter = &block->next; *iter && !(*iter)->free_list; 
	    iter = &(*iter)->next);
	if (*iter && (*iter)->free_list)
	    block = *iter;
	else
	    block = *iter = mem_cache_block_create(cache->item_size);
	item = block->free_list;
    }

    next = (char *)*((uint_ptr_t *)item);
    block->free_list = next;
    tp_debug(("Allocated %p\n", item));
    return (void *)item;
}

static inline int is_in_cache_block(mem_cache_block_t *block, void *ptr,
    int item_size)
{
    char *item = ptr, *start = (char *)(block + 1);

    return item >= start && item < (start + item_size * NUM_ITEMS);
}

static inline mem_cache_block_t *mem_cache_block_find(mem_cache_block_t *block,
    void *ptr, int item_size)
{
    for (; block && !is_in_cache_block(block, ptr, item_size); 
	block = block->next);
    return block;
}

void mem_cache_free(mem_cache_t *cache, void *ptr)
{
    char *item = ptr;
    mem_cache_block_t *block;

    block = mem_cache_block_find(cache->head, ptr, cache->item_size);
    tp_debug(("freeing %p from cache %p\n", item, cache));
    *((uint_ptr_t *)item) = (uint_ptr_t)block->free_list;
    block->free_list = item;
}
