/* copyright (c) 2013, eyal birger
 * all rights reserved.
 * 
 * redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * the name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * this software is provided by the copyright holders and contributors "as is" and
 * any express or implied warranties, including, but not limited to, the implied
 * warranties of merchantability and fitness for a particular purpose are
 * disclaimed. in no event shall <copyright holder> be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential damages
 * (including, but not limited to, procurement of substitute goods or services;
 * loss of use, data, or profits; or business interruption) however caused and
 * on any theory of liability, whether in contract, strict liability, or tort
 * (including negligence or otherwise) arising in any way out of the use of this
 * software, even if advised of the possibility of such damage.
 */
#ifndef __JS_JIT_H__
#define __JS_JIT_H__

typedef struct js_jit_t js_jit_t;

#ifdef CONFIG_JIT

#include <js/js_scan.h>

void jit_init(void);
void jit_uninit(void);
js_jit_t *jit_statement_list(scan_t *scan);
void jit_free(js_jit_t *j);
void jit_call(js_jit_t *j);

#else

static inline void jit_init(void) { }
static inline void jit_uninit(void) { }
static inline void jit_call(js_jit_t *j) { }
static inline js_jit_t *jit_statement_list(scan_t *scan)
{
    return NULL;
}
static inline void jit_free(js_jit_t *j) { }

#endif

#endif
