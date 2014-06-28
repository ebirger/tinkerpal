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
#include "js/js_eval_common.h"
#include "js/js_scan.h"
#include "js/js_obj.h"

void skip_expression(scan_t *scan)
{
    while (CUR_TOK(scan) != TOK_END_STATEMENT && 
        CUR_TOK(scan) != TOK_CLOSE_PAREN && CUR_TOK(scan) != TOK_COMMA && 
        CUR_TOK(scan) != TOK_COLON && CUR_TOK(scan) != TOK_EOF)
    {
        if (CUR_TOK(scan) == TOK_OPEN_PAREN)
        {
            js_scan_next_token(scan);
            skip_expression(scan);
            js_scan_match(scan, TOK_CLOSE_PAREN);
            continue;
        }

        if (CUR_TOK(scan) == TOK_QUESTION)
        {
            js_scan_next_token(scan);
            skip_expression(scan);
            js_scan_match(scan, TOK_COLON);
            skip_expression(scan);
            continue;
        }

        js_scan_next_token(scan);
    }
}

