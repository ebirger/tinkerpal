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
#include "main/console.h"
#include "util/history.h"
#include "util/tp_misc.h"

static history_t *history;
static char test_buf[CONFIG_CLI_BUFFER_SIZE];

static int history_cur_cmp(const char *s)
{
    int hlen;

    hlen = history_get(history, test_buf, sizeof(test_buf));
    if (hlen != strlen(s) || memcmp(s, test_buf, hlen))
    {
        console_printf("(%d) %s != %s\n", hlen, s, test_buf);
        return -1;
    }
    return 0;
}

static void history_commit_str(const char *s)
{
    tstr_t t;

    tstr_init_copy_string(&t, s);
    history_commit(history, &t);
    tstr_free(&t);
}

static int history_test(void)
{
    int rc = 0, rc2 = 0;
    const char *test_strings[] = {
        "Hello World",
        "What's up",
        "Test string",
        NULL
    }, **s;

    console_printf("Starting History Unit Test\n");

    /* Populate history */
    history = history_new();
    for (s = test_strings; *s; s++)
        history_commit_str(*s);
    history_dump(history);

    console_printf("history_prev() test: ");
    rc2 = 0;
    for (history_prev(history), s = test_strings + ARRAY_SIZE(test_strings) - 2;
        s >= test_strings && !(rc2 = history_cur_cmp(*s));
        s--, history_prev(history));
    console_printf("%s\n", rc2 ? "Fail" : "Pass");
    rc |= rc2;

    console_printf("history_next() test: ");
    rc2 = 0;
    for (s = test_strings;
        *s && !(rc2 = history_cur_cmp(*s));
        s++, history_next(history));
    console_printf("%s\n", rc2 ? "Fail" : "Pass");
    rc |= rc2;

    history_free(history);
    console_printf("History Unit Test: %s\n", rc ? "Fail" : "Pass");
    return rc;
}

void app_start(int argc, char *argv[])
{
    console_printf("Application - Unit Tests\n");
    history_test();
}
