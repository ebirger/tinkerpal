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
static int jit_expression(scan_t *scan);

#define JIT_FUNC_CALL1(func, arg1) do { \
    tp_out(("JIT: CALL %s(%s:%d)\n", #func, #arg1, arg1)); \
} while(0)

static int jit_num_new(tnum_t num)
{
    /* XXX: Support FP */
    if (NUMERIC_IS_FP(num))
       return -1;

    JIT_FUNC_CALL1(num_new_int, NUMERIC_INT(num));
    return 0;
}

static int jit_atom(scan_t *scan)
{
    token_type_t tok = CUR_TOK(scan);
    int rc = 0;

    switch (tok)
    {
    case TOK_NUM:
        {
            tnum_t num;

            if (js_scan_get_num(scan, &num))
                return -1;

            if ((rc = jit_num_new(num)))
                return rc;
        }
        break;
    case TOK_OPEN_PAREN:
    default:
        return -1;
    }

    return rc;
}

#define GEN_JIT(name, condition, lower) \
static int name(scan_t *scan) \
{ \
    token_type_t tok; \
    if (lower(scan)) \
        return -1; \
    tok = CUR_TOK(scan); \
    while (condition) \
    { \
        js_scan_next_token(scan); \
        if (lower(scan)) \
            return -1; \
        JIT_FUNC_CALL1(obj_do_op, tok); \
        tok = CUR_TOK(scan); \
    } \
    return 0; \
}

GEN_JIT(jit_factor, (tok == TOK_DIV || tok == TOK_MULT || tok == TOK_MOD),
    jit_atom)
GEN_JIT(jit_term, (tok == TOK_PLUS || tok == TOK_MINUS), jit_factor)

static int jit_expression(scan_t *scan)
{
    return jit_term(scan);
}

int jit_statement_list(scan_t *scan)
{
    int rc;
    scan_t *scan_copy;

    scan_copy = js_scan_save(scan);

    rc = jit_expression(scan_copy);

    js_scan_free(scan_copy);
    tp_out(("JIT Status: %s\n", rc ? "Failed" : "Success"));
    return rc;
}
