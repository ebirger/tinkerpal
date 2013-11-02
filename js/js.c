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
#include <stdio.h> /* For NULL */
#include "util/tmalloc.h"
#include "util/tprintf.h"
#include "main/console.h"
#include "js/js.h"
#include "js/js_eval.h"
#include "js/js_scan.h"
#include "js/js_obj.h"
#include "js/js_builtins.h"
#ifdef CONFIG_GPIO
#include "drivers/gpio/gpio.h"
#endif
#ifdef CONFIG_PLAT_HAS_SERIAL
#include "drivers/serial/serial.h"
#endif
#ifdef CONFIG_SPI
#include "drivers/spi/spi.h"
#endif

obj_t *global_env;

static int js_get_constants_cb(int *constant, tstr_t *s)
{
    /* Additional constants will be registered here */

#ifdef CONFIG_GPIO
    if (!gpio_get_constant(constant, TPTR(s), s->len))
	return 0;
#endif
#ifdef CONFIG_PLAT_HAS_SERIAL
    if (!serial_get_constant(constant, TPTR(s), s->len))
	return 0;
#endif
#ifdef CONFIG_SPI
    if (!spi_get_constant(constant, TPTR(s), s->len))
	return 0;
#endif

    return -1;
}

static void obj_tprintf_handler(printer_t *printer, void *o)
{
    obj_dump(printer, (obj_t *)o);
}

void js_uninit(void)
{
    /* Release the global env without regarding reference counting since we
     * want to tear it down.
     */
    _obj_put(global_env);
    js_builtins_uninit();
    js_eval_uninit();
}

void js_init(void)
{
    tprintf_register_handler('o', obj_tprintf_handler);
    js_scan_set_constants_cb(js_get_constants_cb);
    global_env = env_new(NULL);
    js_eval_init();
    js_builtins_init();
    tp_info(("Object sizes:\n"));
#define OSIZE(o) tp_info((#o ": %d\n", sizeof(o)))
    OSIZE(obj_t);
    OSIZE(function_t);
    OSIZE(num_t);
    OSIZE(string_t);
}
