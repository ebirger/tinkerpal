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
#include "util/tmalloc.h"
#include "util/event.h"
#include "util/debug.h"
#include "js/js_obj.h"
#include "drivers/gpio/gpio.h"

#define Sexception_gpio_pin_mode_unavail \
    S("Exception: Pin mode is unavailable")

int do_digital_write(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    tp_assert(argc == 3);

    if (is_array(argv[1]))
    {
	int value = obj_get_int(argv[2]);
	array_iter_t iter;

	array_iter_init(&iter, argv[1], 1);
	while (array_iter_next(&iter))
	{
	    int pin = obj_get_int(iter.obj);

	    if (gpio_set_pin_mode(pin, GPIO_PM_OUTPUT))
	    {
		array_iter_uninit(&iter);
		goto PinModeError;
	    }

	    gpio_digital_write(pin, value & 1);
	    value >>= 1;
	}
	array_iter_uninit(&iter);
    }
    else
    {
	int pin = obj_get_int(argv[1]);

	if (gpio_set_pin_mode(pin, GPIO_PM_OUTPUT))
	    goto PinModeError;

	gpio_digital_write(pin, obj_true(argv[2]));
    }
    
    *ret = UNDEF;
    return 0;

PinModeError:
    return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);
}

int do_digital_pulse(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int pin;

    tp_assert(argc == 4);

    pin = obj_get_int(argv[1]);

    if (gpio_set_pin_mode(pin, GPIO_PM_OUTPUT))
	goto PinModeError;

    gpio_digital_pulse(pin, obj_true(argv[2]), obj_get_fp(argv[3]));
    
    *ret = UNDEF;
    return 0;

PinModeError:
    return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);
}

int do_digital_read(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int value = 0;

    tp_assert(argc == 2);

    if (is_array(argv[1]))
    {
	array_iter_t iter;

	array_iter_init(&iter, argv[1], 0);
	while (array_iter_next(&iter))
	{
	    int pin = obj_get_int(iter.obj);

	    if (gpio_set_pin_mode(pin, GPIO_PM_INPUT_PULLUP))
	    {
		array_iter_uninit(&iter);
		goto PinModeError;
	    }

	    value |= gpio_digital_read(pin);
	    value <<= 1;
	}
	array_iter_uninit(&iter);
    }
    else
    {
	int pin = obj_get_int(argv[1]);

	if (gpio_set_pin_mode(pin, GPIO_PM_INPUT_PULLUP))
	    goto PinModeError;

	value = gpio_digital_read(obj_get_int(argv[1]));
    }

    *ret = num_new_int(value);
    return 0;

PinModeError:
    return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);
}

int do_analog_write(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int pin;
    double value;

    tp_assert(argc == 3);

    pin = obj_get_int(argv[1]);
    value = obj_get_fp(argv[2]);

    tp_info(("%s: pin %d value %lf\n", __FUNCTION__, pin, value));

    if (gpio_set_pin_mode(pin, GPIO_PM_OUTPUT_ANALOG))
	return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    gpio_analog_write(pin, value);
    *ret = UNDEF;
    return 0;
}

int do_analog_read(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int pin;
    double value;

    tp_assert(argc == 2);

    pin = obj_get_int(argv[1]);

    if (gpio_set_pin_mode(pin, GPIO_PM_INPUT_ANALOG))
	return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    value = gpio_analog_read(pin);

    tp_info(("%s: pin %d value %lf\n", __FUNCTION__, pin, value));

    *ret = num_new_fp(value);
    return 0;
}
#if 0
static int do_pinmode(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int pin, mode;

    tp_assert(argc == 3);

    pin = obj_get_int(argv[1]);
    mode = obj_get_int(argv[2]);

    tp_info(("%s: pin %d mode %d\n", __FUNCTION__, pin, mode));

    /* XXX: validity check on pin mode - pass the enum to the platform function
     */
    if (gpio_set_pin_mode(pin, mode))
	return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    *ret = UNDEF;
    return 0;
}
#endif

#define Swatch_func S("watch_func")
#define Swatch_this S("watch_this")
#define Swatches S("watches")

typedef struct {
    event_t e; /* Must be first */
    obj_t *watch_obj;
} set_watch_work_t;

extern obj_t *meta_env;

static void watch_register(obj_t *watch_obj, int id)
{
    obj_t *watches;

    watches = obj_get_property(NULL, meta_env, &Swatches);
    obj_set_int_property(watches, id, watch_obj);
    obj_put(watches);
}

static void delayed_work_free(event_t *e)
{
    set_watch_work_t *w = (set_watch_work_t *)e;
    obj_put(w->watch_obj);
    tfree(w);
}

static set_watch_work_t *set_watch_work_new(obj_t *func, obj_t *this,
    void (*watch_event)(event_t *e, int resource_id))
{
    set_watch_work_t *w = tmalloc_type(set_watch_work_t);

    w->e.free = delayed_work_free;
    w->e.trigger = watch_event;
    w->watch_obj = object_new();
    obj_set_property(w->watch_obj, Swatch_func, func);
    obj_set_property(w->watch_obj, Swatch_this, this);
    return w;
}

static void set_watch_on_change_cb(event_t *e, int id)
{
    set_watch_work_t *w = (set_watch_work_t *)e;
    obj_t *o, *this, *func;

    func = obj_get_property(NULL, w->watch_obj, &Swatch_func);
    this = obj_get_property(NULL, w->watch_obj, &Swatch_this);
    function_call(&o, this, 1, &func);

    obj_put(func);
    obj_put(this);
    obj_put(o);
}

int do_set_watch(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    set_watch_work_t *w;
    int event_id;

    tp_assert(argc == 3);
    w = set_watch_work_new(argv[1], this, set_watch_on_change_cb);

    event_id = event_watch_set(obj_get_int(argv[2]), &w->e);
    *ret = num_new_int(event_id);
    watch_register(w->watch_obj, event_id);
    return 0;
}

int do_list_watches(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    obj_t *watches = obj_get_property(NULL, meta_env, &Swatches);

    tp_out(("Watch list:\n%o\n", watches));
    
    obj_put(watches);
    return 0;
}

void js_gpio_uninit(void)
{
}

void js_gpio_init(void)
{
    obj_set_property(meta_env, Swatches, array_new());
}
