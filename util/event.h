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
#ifndef __EVENT_H__
#define __EVENT_H__

#include "util/tp_types.h"

typedef struct event_t event_t;

struct event_t {
    void (*trigger)(event_t *e, u32 resource_id, u32 timestamp);
    void (*free)(event_t *e);
};

/* num_timestamps must be a power of 2 */
int _event_watch_set(u32 resource_id, event_t *e, u8 num_timestamps);

static inline int event_watch_set(u32 resource_id, event_t *e)
{
    return _event_watch_set(resource_id, e, 0);
}

void event_watch_del(int watch_id);
void event_watch_del_by_resource(u32 resource_id);
void event_watch_del_all(void);
void event_watch_trigger(u32 resource_id);

int event_timer_set(int ms, event_t *e);
int event_timer_set_period(int ms, event_t *e);
void event_timer_del(int id);
void event_timer_del_all(void);

void event_loop(void);

#endif

