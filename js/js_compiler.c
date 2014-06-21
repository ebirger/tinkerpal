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
#include "js/js_compiler.h"
#include "js/js_obj.h"
#include "js/js_types.h"
#include "js/js_utils.h"
#include "mem/mem_cache.h"
#include "util/tnum.h"
#include "util/tp_types.h"

typedef struct js_jit_t js_jit_t;
static mem_cache_t *jit_mem_cache;

struct js_jit_t {
    void *buffer;
};

#define ARM_THM_JIT_MAX_OPS_NUM 64
#define JIT_MEM_CACHE_ITEM_SIZE (ARM_THM_JIT_MAX_OPS_NUM * sizeof(u16))

static u16 *cur_jit_buffer;
static int cur_jit_buffer_idx;

static int jit_op16(u16 op)
{
    tp_debug(("JIT: OP 0x%x\n", op));

    cur_jit_buffer[cur_jit_buffer_idx] = op;
    cur_jit_buffer_idx++;
    if (cur_jit_buffer_idx == ARM_THM_JIT_MAX_OPS_NUM)
    {
        /* TODO: alloc next buffer, add jump instruction to it */
        tp_err(("JIT: too many ops\n"));
        return -1;
    }

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
#define JIT_INIT(buffer) do { \
    cur_jit_buffer = buffer; \
    cur_jit_buffer_idx = 0; \
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

static int arm_jit_init(void *buf)
{
    cur_jit_buffer = buf;
    cur_jit_buffer_idx = 0;
    ARM_THM_JIT_PUSH(1, (1<<R4));
    return 0;
}

static int arm_jit_uninit(void)
{
    ARM_THM_JIT_POP(1, (1<<R0)|(1<<R4));
    return 0;
}

void jit_call(js_jit_t *j)
{
    obj_t *(*func)(void);

    /* '1' in LSB denotes thumb function call */
    func = (obj_t *(*)(void))((u8 *)j + 1);
    tp_out(("Result %o\n", func()));
}

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

static int jit_expression(scan_t *scan);

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

static obj_t *jit_get_property_helper(obj_t *property)
{
    extern obj_t *cur_env;
    obj_t *o;
    tstr_t prop_name;

    prop_name = to_string(property)->value;

    o = obj_get_property(NULL, cur_env, &prop_name);

    /* XXX: 'property' should be stored when assignement is implemented */
    obj_put(property);
    return o ? o : UNDEF;
}

static int jit_member(scan_t *scan)
{
    token_type_t tok = CUR_TOK(scan);

    if (jit_atom(scan))
        return -1;

    if (tok == TOK_ID)
        JIT_FUNC_CALL1(jit_get_property_helper);

    return 0;
}

static obj_t *jit_function_call_helper(obj_t *func)
{
    extern obj_t *global_env;
    function_args_t args;
    obj_t *argv[1];
    obj_t *ret = UNDEF;
    int rc;

    /* XXX: assert is function */
    argv[0] = func;
    args.argc = 1;
    args.argv = argv;

    rc = function_call(&ret, global_env, args.argc, args.argv);
    if (rc == COMPLETION_RETURN)
        rc = 0;

    return ret;
}

static int jit_function_call(scan_t *scan)
{
    js_scan_match(scan, TOK_OPEN_PAREN);
    if (CUR_TOK(scan) != TOK_CLOSE_PAREN)
        return -1;

    js_scan_match(scan, TOK_CLOSE_PAREN);

    JIT_FUNC_CALL1(jit_function_call_helper);
    return 0;
}

static int jit_functions(scan_t *scan)
{
    if (jit_member(scan))
        return -1;
    
    while (CUR_TOK(scan) == TOK_OPEN_PAREN)
    {
        if (jit_function_call(scan))
            return -1;
    }
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
    jit_functions)
GEN_JIT(jit_term, (tok == TOK_PLUS || tok == TOK_MINUS), jit_factor)

static int jit_expression(scan_t *scan)
{
    return jit_term(scan);
}

void jit_free(js_jit_t *j)
{
    mem_cache_free(jit_mem_cache, j);
}

js_jit_t *jit_statement_list(scan_t *scan)
{
    scan_t *scan_copy;
    void *buffer;
    int rc;

    buffer = mem_cache_alloc(jit_mem_cache);

    scan_copy = js_scan_save(scan);

    if ((rc = arm_jit_init(buffer)))
        goto Exit;

    if ((rc = jit_expression(scan_copy)))
        goto Exit;

    if ((rc = arm_jit_uninit()))
        goto Exit;

Exit:
    js_scan_free(scan_copy);

    tp_out(("JIT Status: %s\n", rc ? "Failed" : "Success"));
    if (rc)
    {
        jit_free(buffer);
        return NULL;
    }
    return buffer;
}

int js_compile(obj_t **po)
{
    function_t *f;

    if (!is_function(*po))
        return throw_exception(po, &S("Only functions compilation for now"));

    f = to_function(*po);

    if (!f->code)
    {
        /* Built-in functions are already compiled. Nothing to do */
        return 0;
    }

    /* First parameter is function name, always exists */
    if (f->formal_params->next)
    {
        return throw_exception(po, &S("Functions with parameters cannot be "
            "compiled yet"));
    }

    return 0;
}

void js_compiler_uninit(void)
{
    mem_cache_destroy(jit_mem_cache);
}

void js_compiler_init(void)
{
    jit_mem_cache = mem_cache_create(JIT_MEM_CACHE_ITEM_SIZE, "JIT");
}
