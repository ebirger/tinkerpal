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
#include "util/event.h"
#include "util/debug.h"
#include "main/console.h"
#include "js/js_obj.h"
#include "js/js_utils.h"
#include "js/jsapi_decl.h"
#include "drivers/spi/spi.h"
#ifdef CONFIG_GPIO
#include "drivers/gpio/gpio.h"
#endif

#define Sspi_id S("spi_id")

/* XXX: provide CS */

static resource_t get_spi_id(obj_t *o)
{
    int ret = -1;
    
    tp_assert(!obj_get_property_int(&ret, o, &Sspi_id));
    return ret;
}

int do_spi_receive(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    *ret = num_new_int(spi_receive(get_spi_id(this)));
    return 0;
}

int do_spi_send(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
#ifdef CONFIG_GPIO
    resource_t cs = 0;
#endif

    if (argc < 2)
        return js_invalid_args(ret);

#ifdef CONFIG_GPIO
    if (argc == 3)
    {
        cs = obj_get_int(argv[2]);

        if (gpio_set_pin_mode(cs, GPIO_PM_OUTPUT))
            return js_invalid_args(ret);
    }

    if (cs)
        gpio_digital_write(cs, 0);
#endif

    if (is_num(argv[1]))
        spi_send(get_spi_id(this), obj_get_int(argv[1]));
    else if (is_array(argv[1]))
    {
        array_iter_t iter;

        array_iter_init(&iter, argv[1], 0);
        while (array_iter_next(&iter))
            spi_send(get_spi_id(this), obj_get_int(iter.obj));
        array_iter_uninit(&iter);
    }
    else
        return js_invalid_args(ret);

#ifdef CONFIG_GPIO
    if (cs)
        gpio_digital_write(cs, 1);
#endif

    *ret = UNDEF;
    return 0;
}

int do_spi_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    resource_t id;

    if (argc != 2)
        return js_invalid_args(ret);

    id = obj_get_int(argv[1]);
    *ret = object_new();
    obj_inherit(*ret, argv[0]);
    obj_set_property_int(*ret, Sspi_id, id);
    spi_init(id);
    return 0;
}
