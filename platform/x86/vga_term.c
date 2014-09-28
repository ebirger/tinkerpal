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
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    COLOR_BLACK = 0,
    COLOR_BLUE = 1,
    COLOR_GREEN = 2,
    COLOR_CYAN = 3,
    COLOR_RED = 4,
    COLOR_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GREY = 7,
    COLOR_DARK_GREY = 8,
    COLOR_LIGHT_BLUE = 9,
    COLOR_LIGHT_GREEN = 10,
    COLOR_LIGHT_CYAN = 11,
    COLOR_LIGHT_RED = 12,
    COLOR_LIGHT_MAGENTA = 13,
    COLOR_LIGHT_BROWN = 14,
    COLOR_WHITE = 15,
} vga_color_t;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR(fg, bg) ((fg) | (bg) << 4)
#define IDX(col, row) ((row) * VGA_WIDTH + (col))

static uint8_t term_row, term_column;
static uint8_t term_color = VGA_COLOR(COLOR_GREEN, COLOR_BLACK);
static uint16_t *term_buffer = (uint16_t *)0xb8000;

static inline void term_charat(char c, uint8_t color, uint8_t x, uint8_t y)
{
    term_buffer[IDX(x, y)] = c | ((uint16_t)color << 8);
}

static void term_newline(void)
{
    uint16_t *src;

    if (term_row < VGA_HEIGHT - 1)
    {
        term_row++;
        return;
    }

    /* Scroll up by naive copy */
    src = term_buffer + IDX(0, 1);
    memmove(term_buffer, src, (VGA_HEIGHT - 1) * VGA_WIDTH * sizeof(uint16_t));
}

void vga_term_init(void)
{
    uint8_t x, y;

    /* Clear screen */
    for (y = 0; y < VGA_HEIGHT; y++)
    {
        for (x = 0; x < VGA_WIDTH; x++)
            term_charat(' ', term_color, x, y);
    }
}

void vga_term_putchar(char c)
{
    if (c == '\r')
        term_column = 0;
    else if (c == '\n')
        term_newline();
    else
    {
        term_charat(c, term_color, term_column, term_row);
        if (++term_column == VGA_WIDTH)
        {
            term_column = 0;
            term_newline();
        }
    }
}
