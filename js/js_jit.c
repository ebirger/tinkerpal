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
#include "js/js_jit.h"
#include "js/js_obj.h"
#include "util/tnum.h"
#include "util/tp_types.h"

static int jit_expression(scan_t *scan);

#define ARM_THUMB
#ifdef ARM_THUMB

#define ARM_THM_JIT_MAX_OPS_NUM 128

static u16 jit_buffer[ARM_THM_JIT_MAX_OPS_NUM];
static int jit_buffer_idx;

static int jit_op16(u16 op)
{
    tp_debug(("JIT: OP 0x%x\n", op));

    jit_buffer[jit_buffer_idx] = op;
    jit_buffer_idx++;
    if (jit_buffer_idx == ARM_THM_JIT_MAX_OPS_NUM)
        return -1;

    return 0;
}

static int jit_op32(u32 op)
{
    if (jit_op16(op >> 16))
        return -1;

    return jit_op16(op & 0xffff);
}

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define SP 13

/* op: 0 - push, 1 - pop
 * R: 1 - operate on lr too
 * register_list: bitmap of registers
 */
#define ARM_THM_JIT_PUSH_POP(op, R, register_list) do { \
    tp_info(("JIT: push_pop op %d, R %d, register_list %x\n", op, R, \
        register_list)); \
    if (jit_op16(0xb000 | ((op)<<11) | (1<<10) | ((R)<<8) | (register_list))) \
        return -1; \
} while(0)

#define ARM_THM_JIT_PUSH(R, register_list) \
    ARM_THM_JIT_PUSH_POP(0, R, register_list)
#define ARM_THM_JIT_POP(R, register_list) \
    ARM_THM_JIT_PUSH_POP(1, R, register_list)

#define ARM_THM_JIT_MOV(ld, imm8) do { \
    if (imm8 > 255) \
        return -1; \
    tp_info(("JIT: mov ld %d, imm8 %x\n", ld, imm8)); \
    if (jit_op16(0x2000 | ((ld)<<8) | (imm8))) \
        return -1; \
} while(0)

#define ARM_THM_JIT_MOV_REG(rd, rm) do { \
    tp_info(("JIT: mov rd %d, rm %d\n", rd, rm)); \
    if (jit_op16(0x4600 | ((rm)<<3) | (rd))) \
        return -1; \
} while(0)

#define _ARM_THM_JIT_MOVW(i, imm4, imm3, rd, imm8) do { \
    u32 op32; \
    tp_info(("JIT: movw i %d, imm4 %x, imm3 %x, rd %d, imm8 %x\n", i, imm4, \
        imm3, rd, imm8)); \
    op32 = 0xf000 | ((i)<<10) | (1<<9) | (1<<6) | (imm4); \
    op32 <<= 16; \
    op32 |= ((imm3)<<12) | ((rd)<<8) | (imm8); \
    if (jit_op32(op32)) \
        return -1; \
} while(0)

#define _ARM_THM_JIT_MOVT(i, imm4, imm3, rd, imm8) do { \
    u32 op32; \
    tp_info(("JIT: movt i %d, imm4 %x, imm3 %x, rd %d, imm8 %x\n", i, imm4, \
        imm3, rd, imm8)); \
    op32 = 0xf000 | ((i)<<10) | (1<<9) | (1<<7) | (1<<6) | (imm4); \
    op32 <<= 16; \
    op32 |= ((imm3)<<12) | ((rd)<<8) | (imm8); \
    if (jit_op32(op32)) \
        return -1; \
} while(0)

#define ARM_THM_JIT_MOVW(rd, imm16) do { \
    tp_info(("JIT: movw rd %d, imm16 %x\n", rd, imm16)); \
    _ARM_THM_JIT_MOVW(((imm16) >> 11) & 1, ((imm16)>>12) & 0xf, ((imm16) >> 8) & 0x7, \
        rd, (imm16) & 0xff); \
} while(0) 

#define ARM_THM_JIT_MOVT(rd, imm16) do { \
    tp_info(("JIT: movt rd %d, imm16 %x\n", rd, imm16)); \
    _ARM_THM_JIT_MOVT(((imm16) >> 11) & 1, ((imm16)>>12) & 0xf, ((imm16) >> 8) & 0x7, \
        rd, (imm16) & 0xff); \
} while(0)

#define ARM_THM_JIT_BLX(rm) do { \
    tp_info(("JIT: blx rm %d\n", rm)); \
    if (jit_op16(0x4000 | (0xf<<7) | ((rm)<<3))) \
        return -1; \
} while(0)

#define ARM_THM_JIT_REG_SET(r, val) do { \
    tp_info(("JIT: reg set %d = %s:%x\n", r, #val, (u32)val)); \
    ARM_THM_JIT_MOVW(r, (val) & 0xffff); \
    ARM_THM_JIT_MOVT(r, ((val)>>16) & 0xffff); \
} while(0)

#define ARM_THM_JIT_CALL(addr) do { \
    ARM_THM_JIT_REG_SET(R4, (u32)(addr) | 0x1); \
    ARM_THM_JIT_BLX(R4); \
} while(0)

/* API */
#define JIT_INIT() do { \
    jit_buffer_idx = 0; \
    ARM_THM_JIT_PUSH(1, (1<<R4)); \
} while(0)

#define JIT_UNINIT() do { \
    ARM_THM_JIT_POP(1, (1<<R0)|(1<<R4)); \
} while(0)

#define JIT_FUNC_CALL0(func) do { \
    ARM_THM_JIT_CALL(func); \
    ARM_THM_JIT_PUSH(0, (1<<R0)); \
} while(0)

#define JIT_FUNC_CALL1(func) do { \
    ARM_THM_JIT_POP(0, 1<<R0); \
    JIT_FUNC_CALL0(func); \
} while(0)

#define JIT_FUNC_CALL1_ARG(func, arg) do { \
    ARM_THM_JIT_MOV(R0, arg); \
    JIT_FUNC_CALL0(func); \
} while(0)

#define JIT_FUNC_CALL2_ARG(func, arg) do { \
    ARM_THM_JIT_POP(0, (1<<R2)); \
    ARM_THM_JIT_POP(0, (1<<R1)); \
    ARM_THM_JIT_MOV(R0, arg); \
    JIT_FUNC_CALL0(func); \
} while(0)

/* Return value in <R0, R1> */
#define JIT_FUNC_CALL2_RET(func, arg1, arg2) do { \
    ARM_THM_JIT_REG_SET(R1, arg1); \
    ARM_THM_JIT_REG_SET(R2, arg2); \
    /* Make space for return value */ \
    ARM_THM_JIT_PUSH(0, (1<<R1) | (1<<R2)); \
    ARM_THM_JIT_MOV_REG(R0, SP); /* values pointer */ \
    JIT_FUNC_CALL0(func); \
    ARM_THM_JIT_POP(0, 1<<R0); /* Don't need the return value */ \
    /* Fetch the dupped tstr from the stack */ \
    ARM_THM_JIT_POP(0, 1<<R0); \
    ARM_THM_JIT_POP(0, 1<<R1); \
} while(0)

static void jit_call(void)
{
    obj_t *(*func)(void);
    u8 *buf;

    buf = tmalloc(jit_buffer_idx * sizeof(u16), "jit_function");
    memcpy(buf, jit_buffer, jit_buffer_idx * sizeof(u16));

    /* '1' in LSB denotes thumb function call */
    func = (obj_t *(*)(void))(buf + 1);
    tp_out(("Result %o\n", func()));
    tfree(buf);
}

#else

#error JIT not available yet

#endif

static int jit_num_new(tnum_t num)
{
    /* XXX: Support FP */
    if (NUMERIC_IS_FP(num))
       return -1;

    JIT_FUNC_CALL1_ARG(num_new_int, NUMERIC_INT(num));
    return 0;
}

static int jit_string_new(tstr_t str)
{
    u32 *s = (u32 *)&str;

    /* tstr_dup the value so it can be used more than once */
    JIT_FUNC_CALL2_RET(tstr_dup, s[0], s[1]);

    JIT_FUNC_CALL0(string_new);
    return 0;
}

static int jit_atom(scan_t *scan)
{
    token_type_t tok = CUR_TOK(scan);

    switch (tok)
    {
    case TOK_NUM:
        {
            tnum_t num;

            if (js_scan_get_num(scan, &num))
                return -1;

            if (jit_num_new(num))
                return -1;
        }
        break;
    case TOK_STRING:
        {
            tstr_t str;

            if (js_scan_get_string(scan, &str))
                return -1;

            if (jit_string_new(str))
            {
                tstr_free(&str);
                return -1;
            }

            /* XXX: memory leak since we don't provide means to free str */
        }
        break;
    case TOK_ID:
        {
            tstr_t id;

            if (js_scan_get_identifier(scan, &id))
                return -1;

            if (jit_string_new(id))
            {
                tstr_free(&id);
                return -1;
            }

            /* XXX: memory leak since we don't provide means to free str */
        }
        break;
    case TOK_OPEN_PAREN:
        js_scan_next_token(scan);

        if (jit_expression(scan))
            return -1;

        if (_js_scan_match(scan, TOK_CLOSE_PAREN))
            return -1;

        break;
    default:
        return -1;
    }

    return 0;
}

static obj_t *jit_get_property_do(obj_t *property)
{
    extern obj_t *cur_env;
    obj_t *o;
    tstr_t prop_name = obj_get_str(property);

    o = obj_get_property(NULL, cur_env, &prop_name);

    tstr_free(&prop_name);
    /* XXX: 'property' should be stored when assignement is implemented */
    obj_put(property);
    return o ? o : UNDEF;
}

static int jit_get_property(void)
{
    JIT_FUNC_CALL1(jit_get_property_do);
    return 0;
}

static int jit_member(scan_t *scan)
{
    token_type_t tok = CUR_TOK(scan);

    if (jit_atom(scan))
        return -1;

    if (tok == TOK_ID)
        return jit_get_property();

    return 0;
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
        JIT_FUNC_CALL2_ARG(obj_do_op, tok); \
        tok = CUR_TOK(scan); \
    } \
    return 0; \
}

GEN_JIT(jit_factor, (tok == TOK_DIV || tok == TOK_MULT || tok == TOK_MOD),
    jit_member)
GEN_JIT(jit_term, (tok == TOK_PLUS || tok == TOK_MINUS), jit_factor)

static int jit_expression(scan_t *scan)
{
    return jit_term(scan);
}

int jit_statement_list(scan_t *scan)
{
    int rc = 0;
    scan_t *scan_copy;
    scan_copy = js_scan_save(scan);

    JIT_INIT();

    rc = jit_expression(scan_copy);

    JIT_UNINIT();

    if (!rc)
        jit_call();

    js_scan_free(scan_copy);
    tp_out(("JIT Status: %s\n", rc ? "Failed" : "Success"));
    return rc;
}
