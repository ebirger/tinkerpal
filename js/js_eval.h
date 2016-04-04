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
#ifndef __JS_EVAL_H__
#define __JS_EVAL_H__

#include "util/tstr.h"
#include "util/tstr_list.h"
#include "js/js_obj.h"

int js_eval(obj_t **ret, tstr_t *code);
int js_eval_module(obj_t **ret, tstr_t *code);
int js_eval_obj(obj_t **ret, obj_t *obj);
void js_eval_noret(tstr_t *code);

/* Return 'rank' of code - used for multiline edit:
 * Examples:
 *    function f() { => 1
 *    function f() { while(1) { => 2
 * ...
 */
int js_eval_rank(tstr_t code);
/* Stop current execution */
void js_eval_stop_execution(void);

void evaluated_function_code_free(void *code);
int call_evaluated_function(obj_t **ret, obj_t *this_obj, int argc, 
    obj_t *argv[]);
int parse_function_param_list(tstr_list_t **params, scan_t *scan);

void js_eval_uninit(void);
void js_eval_init(void);

#endif
