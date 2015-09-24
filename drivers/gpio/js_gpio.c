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
#include "js/js_obj.h"
#include "js/js_utils.h"
#include "js/js_event.h"
#include "drivers/gpio/gpio.h"

#define Sexception_gpio_pin_mode_unavail \
    S("Exception: Pin mode is unavailable")

int do_digital_write(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    if (argc != 3)
        return js_invalid_args(ret);

    if (is_array(argv[1]))
    {
        int value = obj_get_int(argv[2]);
        array_iter_t iter;

        array_iter_init(&iter, argv[1], ARRAY_ITER_FLAG_REVERSE);
        while (array_iter_next(&iter))
        {
            resource_t pin = obj_get_int(iter.obj);

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
        resource_t pin = obj_get_int(argv[1]);

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
    resource_t pin;

    if (argc != 4)
        return js_invalid_args(ret);

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

    if (argc != 2)
        return js_invalid_args(ret);

    if (is_array(argv[1]))
    {
        array_iter_t iter;

        array_iter_init(&iter, argv[1], 0);
        while (array_iter_next(&iter))
        {
            resource_t pin = obj_get_int(iter.obj);

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
        resource_t pin = obj_get_int(argv[1]);

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
    resource_t pin;
    double value;
    int freq = 1846;

    if (argc != 3 && argc != 4)
        return js_invalid_args(ret);

    if (argc == 4)
    {
	if (!obj_get_property_int(&freq, argv[3], &S("freq")))
	{
	    if (freq < 0)
		return throw_exception(ret, &S("frequency must be positive"));
	}
    }

    pin = obj_get_int(argv[1]);
    value = obj_get_fp(argv[2]);

    tp_info("%s: pin %d value %lf freq %d\n", __FUNCTION__, pin, value, freq);

    if (gpio_set_pin_mode(pin, GPIO_PM_OUTPUT_ANALOG))
        return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    gpio_pwm_start(pin, freq, value * 100);
    *ret = UNDEF;
    return 0;
}

int do_analog_read(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    resource_t pin;
    double value;

    if (argc != 2)
        return js_invalid_args(ret);

    pin = obj_get_int(argv[1]);

    if (gpio_set_pin_mode(pin, GPIO_PM_INPUT_ANALOG))
        return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    value = gpio_analog_read(pin);

    tp_info("%s: pin %d value %lf\n", __FUNCTION__, pin, value);

    *ret = num_new_fp(value);
    return 0;
}
#if 0
static int do_pinmode(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    int pin, mode;

    if (argc != 3)
        return js_invalid_args(ret);

    pin = obj_get_int(argv[1]);
    mode = obj_get_int(argv[2]);

    tp_info("%s: pin %d mode %d\n", __FUNCTION__, pin, mode);

    /* XXX: validity check on pin mode - pass the enum to the platform function
     */
    if (gpio_set_pin_mode(pin, mode))
        return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    *ret = UNDEF;
    return 0;
}
#endif

static void set_watch_trigger(event_t *e, u32 id, u64 timestamp)
{
    obj_t *data_obj;

    data_obj = object_new();
    obj_set_property_fp(data_obj, S("timestamp"), (double)timestamp / 1000000);
    obj_set_property_int(data_obj, S("state"), gpio_digital_read(id));

    _js_event_gen_trigger(e, id, data_obj);

    obj_put(data_obj);
}

int do_set_watch(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    event_t *e;
    int event_id, qlen = 1;
    resource_t pin;

    if (argc != 3 && argc != 4)
        return js_invalid_args(ret);

    if (argc == 4)
    {
	if (!obj_get_property_int(&qlen, argv[3], &S("qlen")))
	{
	    if (qlen > 127 || (qlen & (qlen - 1)))
	    {
		return throw_exception(ret,
		    &S("qlen must be <128 and power of 2"));
	    }
	}
    }

    pin = obj_get_int(argv[2]);

    if (gpio_set_pin_mode(pin, GPIO_PM_INPUT_PULLUP))
        return throw_exception(ret, &Sexception_gpio_pin_mode_unavail);

    e = js_event_new(argv[1], this, set_watch_trigger);

    event_id = _event_watch_set(pin, e, qlen, 0);
    *ret = num_new_int(event_id);
    return 0;
}
