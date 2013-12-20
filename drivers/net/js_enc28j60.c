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
#include "js/js_obj.h"
#include "js/js_utils.h"
#include "js/js_event.h"
#include "net/js_etherif.h"
#include "drivers/net/enc28j60.h"

int do_enc28j60_packet_recv(obj_t **ret, obj_t *this, int argc,
    obj_t *argv[])
{
    int size;
    obj_t *array_buffer;
    etherif_t *ethif = etherif_obj_get_etherif(this);

    size = ethif->ops->packet_size(ethif);
    array_buffer = array_buffer_new(size);
    size = ethif->ops->packet_recv(ethif, (u8 *)
	TPTR(&((array_buffer_t *)array_buffer)->value), size);
    *ret = array_buffer_view_new(array_buffer, 
	ABV_SHIFT_8_BIT | ABV_FLAG_UNSIGNED, 0, size);
    obj_put(array_buffer);
    return 0;
}

int do_enc28j60_on_packet_received(obj_t **ret, obj_t *this, int argc,
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

int do_enc28j60_on_port_change(obj_t **ret, obj_t *this, int argc,
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

int do_enc28j60_link_status(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    etherif_t *ethif = etherif_obj_get_etherif(this);

    *ret = ethif->ops->link_status(ethif) ? TRUE : FALSE;
    return 0;
}

int do_enc28j60_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int spi_port, cs, intr;
    etherif_t *ethif;

    if (argc != 4)
	return js_invalid_args(ret);

    spi_port = obj_get_int(argv[1]);
    cs = obj_get_int(argv[2]);
    intr = obj_get_int(argv[3]);

    ethif = enc28j60_new(spi_port, cs, intr);
    *ret = etherif_obj_new(ethif);
    obj_inherit(*ret, argv[0]);
    return 0;
}
