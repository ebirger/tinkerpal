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
#include "util/tp_misc.h"
#include "util/event.h"
#include "util/debug.h"
#include "drivers/resources.h"
#include "drivers/lcd/sdl_screen.h"
#include "graphics/colors.h"
#include "SDL.h"

typedef struct {
    sdl_screen_params_t params;
    event_t render_timer;
    canvas_t canvas;
    SDL_Surface *surface;
} sdl_screen_t;

static sdl_screen_t g_sdl_screen;

static void render_timer_trigger(event_t *e, u32 resource_id)
{
    sdl_screen_t *screen = container_of(e, sdl_screen_t, render_timer);

    /* Update screen */
    SDL_UpdateRect(screen->surface, 0, 0, screen->params.width,
	screen->params.height);
}

static void sdl_screen_init(sdl_screen_t *screen)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
	tp_err(("Unable to init SDL: %s\n", SDL_GetError()));
	return;
    }

    atexit(SDL_Quit);

    screen->surface = SDL_SetVideoMode(screen->params.width,
	screen->params.height, 16, SDL_SWSURFACE);

    if (!screen->surface)
    {
	tp_err(("Unable to set resolution:  %s\n", SDL_GetError()));
	return;
    }

    /* 30 fps */
    event_timer_set_period(1000/30, &screen->render_timer);
}

static void sdl_screen_pixel_set(canvas_t *c, u16 x, u16 y, u16 val)
{
    sdl_screen_t *screen = container_of(c, sdl_screen_t, canvas);
    u16 *ptr = (u16 *)screen->surface->pixels, bgr_val;
    int lineoffset;

    lineoffset = y * (screen->surface->pitch / 2);

    /* SDL works in BGR mode. Swap values here.
     * XXX: this could be better done by swapping the SDL palette 
     */
    bgr_val = (val & COLOR_BLUE) >> COLOR_BLUE_SHIFT;
    bgr_val |= val & COLOR_GREEN;
    bgr_val |= (val & COLOR_RED) << COLOR_BLUE_SHIFT;

    ptr[lineoffset + x] = bgr_val;
}

static const canvas_ops_t sdl_screen_ops = {
    .pixel_set = sdl_screen_pixel_set,
};

canvas_t *sdl_screen_new(const sdl_screen_params_t *params)
{
    sdl_screen_t *screen = &g_sdl_screen;

    screen->params = *params;
    screen->render_timer.trigger = render_timer_trigger;

    sdl_screen_init(screen);

    screen->canvas.width = params->width;
    screen->canvas.height = params->height;
    screen->canvas.ops = &sdl_screen_ops;

    canvas_register(&screen->canvas);
    return &screen->canvas;
}
