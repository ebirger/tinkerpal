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

int do_digital_write(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    tp_assert(argc == 2);

    if (is_array(argv[0]))
    {
	int value = obj_get_int(argv[1]);
	array_iter_t iter;

	array_iter_init(&iter, argv[0], 1);
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
	int pin = obj_get_int(argv[0]);

	if (gpio_set_pin_mode(pin, GPIO_PM_OUTPUT))
	    goto PinModeError;

	gpio_digital_write(pin, obj_true(argv[1]));
    }
    
    *ret = UNDEF;
    return 0;

PinModeError:
    return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);
}

int do_digital_pulse(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    int pin;

    tp_assert(argc == 3);

    pin = obj_get_int(argv[0]);

    if (gpio_set_pin_mode(pin, GPIO_PM_OUTPUT))
	goto PinModeError;

    gpio_digital_pulse(pin, obj_true(argv[1]), obj_get_fp(argv[2]));
    
    *ret = UNDEF;
    return 0;

PinModeError:
    return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);
}

int do_digital_read(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    int value = 0;

    tp_assert(argc == 1);

    if (is_array(argv[0]))
    {
	array_iter_t iter;

	array_iter_init(&iter, argv[0], 0);
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
	int pin = obj_get_int(argv[0]);

	if (gpio_set_pin_mode(pin, GPIO_PM_INPUT_PULLUP))
	    goto PinModeError;

	value = gpio_digital_read(obj_get_int(argv[0]));
    }

    *ret = num_new_int(value);
    return 0;

PinModeError:
    return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);
}

int do_analog_write(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    int pin;
    double value;

    tp_assert(argc == 2);

    pin = obj_get_int(argv[0]);
    value = obj_get_fp(argv[1]);

    tp_info(("%s: pin %d value %lf\n", __FUNCTION__, pin, value));

    if (gpio_set_pin_mode(pin, GPIO_PM_OUTPUT_ANALOG))
	return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    gpio_analog_write(pin, value);
    *ret = UNDEF;
    return 0;
}

int do_analog_read(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    int pin;
    double value;

    tp_assert(argc == 1);

    pin = obj_get_int(argv[0]);

    if (gpio_set_pin_mode(pin, GPIO_PM_INPUT_ANALOG))
	return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    value = gpio_analog_read(pin);

    tp_info(("%s: pin %d value %lf\n", __FUNCTION__, pin, value));

    *ret = num_new_fp(value);
    return 0;
}
#if 0
static int do_pinmode(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    int pin, mode;

    tp_assert(argc == 2);

    pin = obj_get_int(argv[0]);
    mode = obj_get_int(argv[1]);

    tp_info(("%s: pin %d mode %d\n", __FUNCTION__, pin, mode));

    /* XXX: validity check on pin mode - pass the enum to the platform function
     */
    if (gpio_set_pin_mode(pin, mode))
	return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    *ret = UNDEF;
    return 0;
}
#endif

typedef struct {
    event_watch_t ew; /* Must be first */
    int id;
    obj_t *func;
    obj_t *this;
} set_watch_work_t;

static void delayed_work_free(event_watch_t *ew)
{
    set_watch_work_t *w = (set_watch_work_t *)ew;
    obj_put(w->func);
    obj_put(w->this);
    tfree(w);
}

static set_watch_work_t *set_watch_work_new(int id, obj_t *func, obj_t *this,
    void (*watch_event)(event_watch_t *ew, int resource_id))
{
    set_watch_work_t *w = tmalloc_type(set_watch_work_t);

    w->ew.free = delayed_work_free;
    w->ew.watch_event = watch_event;
    w->id = id;
    w->func = obj_get(func);
    w->this = obj_get(this);
    return w;
}

static void set_watch_on_change_cb(event_watch_t *ew, int id)
{
    set_watch_work_t *w = (set_watch_work_t *)ew;
    obj_t *o;

    function_call(&o, to_function(w->func), w->this, 0, NULL);

    obj_put(o);
}

int do_set_watch(obj_t **ret, function_t *func, obj_t *this, 
    int argc, obj_t *argv[])
{
    set_watch_work_t *w;
    int event_id;

    tp_assert(argc == 2);
    w = set_watch_work_new(obj_get_int(argv[1]), argv[0], this, 
	set_watch_on_change_cb);

    event_id = event_watch_set(w->id, &w->ew);
    *ret = num_new_int(event_id);
    return 0;
}
