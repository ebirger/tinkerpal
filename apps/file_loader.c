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
#include <stdio.h>
#include "util/tstr.h"
#include "util/debug.h"
#include "js/js_types.h"
#include "js/js_eval.h"
#include "js/js.h"

/* XXX: this should come from platform layer */
static FILE *fp;
static char buf[128*1024];

void app_start(int argc, char *argv[])
{
    obj_t *o = NULL;
    tstr_t code = {};
    int len;
    
    if (argc != 2)
	tp_crit(("Usage %s <file>\n", argv[0]));

    if (!(fp = fopen(argv[1], "r")))
	tp_crit(("Error reading file %s\n", argv[1]));

    len = fread(buf, 1, sizeof(buf), fp);
    tstr_init(&code, buf, len, 0);
    fclose(fp);

    if (js_eval(&o, &code) == COMPLETION_THROW)
    	tp_crit(("Evaluation resulted in exception %o\n", o));

    obj_put(o);
}
