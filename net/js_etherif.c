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
#include "net/js_etherif.h"
#include "js/js_utils.h"
#include "js/js_event.h"

#define Sether_id S("ether_id")

int etherif_obj_constructor(etherif_t *ethif, obj_t **ret, obj_t *this,
    int argc, obj_t *argv[])
{
    *ret = object_new();
    _obj_set_property(*ret, Sether_id, num_new_int(ethif->id));
    obj_inherit(*ret, argv[0]);
    return 0;
}

etherif_t *etherif_obj_get_etherif(obj_t *o)
{
    int id = -1;
    
    tp_assert(!obj_get_property_int(&id, o, &Sether_id));
    return etherif_get_by_id(id);
}

int do_etherif_packet_recv(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    int size;
    obj_t *array_buffer;
    etherif_t *ethif = etherif_obj_get_etherif(this);

    if (ethif->ops->packet_size)
	size = ethif->ops->packet_size(ethif);
    else
	size = 400; /* Arbitrary, but should be good enough */
    array_buffer = array_buffer_new(size);
    size = ethif->ops->packet_recv(ethif, (u8 *)
	TPTR(&((array_buffer_t *)array_buffer)->value), size);
    if (size <= 0)
    {
	obj_put(array_buffer);
	return throw_exception(ret, &S("Exception: can't read packet"));
    }
    *ret = array_buffer_view_new(array_buffer, 
	ABV_SHIFT_8_BIT | ABV_FLAG_UNSIGNED, 0, size);
    obj_put(array_buffer);
    return 0;
}

int do_etherif_on_packet_received(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    etherif_t *ethif = etherif_obj_get_etherif(this);
    event_t *e;

    if (argc != 2)
	return js_invalid_args(ret);

    e = js_event_new(argv[1], this, js_event_gen_trigger);

    etherif_on_packet_received_event_set(ethif, e);
    *ret = UNDEF;
    return 0;
}

int do_etherif_on_packet_xmit(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    etherif_t *ethif = etherif_obj_get_etherif(this);
    event_t *e;

    if (argc != 2)
	return js_invalid_args(ret);

    e = js_event_new(argv[1], this, js_event_gen_trigger);

    etherif_on_packet_xmit_event_set(ethif, e);
    *ret = UNDEF;
    return 0;
}

int do_etherif_on_port_change(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    etherif_t *ethif = etherif_obj_get_etherif(this);
    event_t *e;

    if (argc != 2)
	return js_invalid_args(ret);

    e = js_event_new(argv[1], this, js_event_gen_trigger);

    etherif_on_port_change_event_set(ethif, e);
    *ret = UNDEF;
    return 0;
}

int do_etherif_link_status(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    etherif_t *ethif = etherif_obj_get_etherif(this);

    *ret = ethif->ops->link_status(ethif) ? TRUE : FALSE;
    return 0;
}