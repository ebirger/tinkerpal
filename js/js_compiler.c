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
#include "js/js_eval_common.h"
#include "mem/mem_cache.h"
#include "util/tnum.h"
#include "util/tp_types.h"

static mem_cache_t *js_compiler_mem_cache;

#define ARM_THM_MAX_OPS_NUM 64
#define MEM_CACHE_ITEM_SIZE ((ARM_THM_MAX_OPS_NUM * sizeof(u16)) + 2)

extern obj_t *cur_env;

static u16 *op_buf;
static int op_buf_index;
static int total_ops;
static int total_blocks;

static void code_block_chain(void);

static u16 s11_to_u16(int s11)
{
    union {
        int _11_bit : 11;
        u16 _16_bit;
    } val;

    val._11_bit = s11;
    return val._16_bit;
}

static void _op16(u16 op)
{
    op_buf[op_buf_index] = op;
    op_buf_index++;
    total_ops++;
}

static int op16(u16 op) __attribute__((noinline));
static int op16(u16 op)
{
    _op16(op);
    if (op_buf_index == ARM_THM_MAX_OPS_NUM - 2)
        code_block_chain();
    return 0;
}

static void op32_prep(void) __attribute__((noinline));
static void op32_prep(void)
{
    if (op_buf_index >= ARM_THM_MAX_OPS_NUM - 3)
        code_block_chain();
}

#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define SP 13

#define OP16(val) do { \
    tp_debug(("%s:%d\t: %x : %s\n", __FUNCTION__, __LINE__, val, #val)); \
    if (op16(val)) \
        return -1; \
} while (0)

#define OP32(hi, lo) do { \
    tp_debug(("%s:%d\t: %x,%x\n", __FUNCTION__, __LINE__, hi, lo)); \
    op32_prep(); \
    if (op16(hi)) \
        return -1; \
    if (op16(lo)) \
        return -1; \
} while (0)

/* ARM Thumb2 instructions encoding */

/* op: 0 - push, 1 - pop
 * R: 1 - operate on lr too
 * register_list: bitmap of registers
 */
#define ARM_THM_PUSH_POP_VAL(op, R, register_list) \
    (0xb000 | ((op)<<11) | (1<<10) | ((R)<<8) | (register_list))
#define ARM_THM_MOV_VAL(ld, imm8) (0x2000 | ((ld)<<8) | (imm8))
#define ARM_THM_STR_VAL(ld, ln, imm5) (0x6000 | ((ld) | (ln)<<3 | ((imm5)<<6)))
#define ARM_THM_MOV_REG_VAL(rd, rm) (0x4600 | ((rm)<<3) | (rd))
#define ARM_THM_BLX_VAL(rm) (0x4000 | (0xf<<7) | ((rm)<<3))
#define ARM_THM_B_PREFIX 0xe000
#define ARM_THM_B_VAL(offs) (ARM_THM_B_PREFIX | s11_to_u16(offs))
#define ARM_THM_ADD_SUB_SP_VAL(op, imm) (0xb000 | ((op)<<7) | ((imm) & 0x7f))
#define ARM_THM_ADD_IMM_VAL(ld, imm) (0x3000 | ((ld)<<8) | ((imm) & 0xff))
#define ARM_THM_MOVW_HI(i, imm4, imm3, rd, imm8) \
    (0xf000 | ((i)<<10) | (1<<9) | (1<<6) | (imm4))
#define ARM_THM_MOVW_LO(i, imm4, imm3, rd, imm8) \
    (((imm3)<<12) | ((rd)<<8) | (imm8))
#define ARM_THM_MOVT_HI(i, imm4, imm3, rd, imm8) \
    (0xf000 | ((i)<<10) | (1<<9) | (1<<7) | (1<<6) | (imm4))
#define ARM_THM_MOVT_LO(i, imm4, imm3, rd, imm8) \
    (((imm3)<<12) | ((rd)<<8) | (imm8))

#define ARM_THM_PUSH_POP(op, R, register_list) \
    OP16(ARM_THM_PUSH_POP_VAL(op, R, register_list))
#define ARM_THM_PUSH(register_list) \
    ARM_THM_PUSH_POP(0, 0, register_list)
#define ARM_THM_POP(register_list) \
    ARM_THM_PUSH_POP(1, 0, register_list)

#define ARM_THM_MOV(ld, imm8) do { \
    if (imm8 > 255) \
        return -1; \
    OP16(ARM_THM_MOV_VAL(ld, imm8)); \
} while(0)

#define ARM_THM_STR(ld, ln, imm5) OP16(ARM_THM_STR_VAL(ld, ln, imm5))

#define ARM_THM_MOV_REG(rd, rm) OP16(ARM_THM_MOV_REG_VAL(rd, rm))

#define _ARM_THM_MOVW(i, imm4, imm3, rd, imm8) do { \
    OP32(ARM_THM_MOVW_HI(i, imm4, imm3, rd, imm8), \
        ARM_THM_MOVW_LO(i, imm4, imm3, rd, imm8)); \
} while(0)

#define _ARM_THM_MOVT(i, imm4, imm3, rd, imm8) do { \
    OP32(ARM_THM_MOVT_HI(i, imm4, imm3, rd, imm8), \
        ARM_THM_MOVT_LO(i, imm4, imm3, rd, imm8)); \
} while(0)

#define ARM_THM_MOVW(rd, imm16) do { \
    _ARM_THM_MOVW(((imm16) >> 11) & 1, ((imm16)>>12) & 0xf, \
        ((imm16) >> 8) & 0x7, rd, (imm16) & 0xff); \
} while(0)

#define ARM_THM_MOVT(rd, imm16) do { \
    _ARM_THM_MOVT(((imm16) >> 11) & 1, ((imm16)>>12) & 0xf, \
        ((imm16) >> 8) & 0x7, rd, (imm16) & 0xff); \
} while(0)

#define ARM_THM_BLX(rm) OP16(ARM_THM_BLX_VAL(rm))

#define ARM_THM_B(offs) OP16(ARM_THM_B_VAL(offs))

#define ARM_THM_REG_SET(r, val) do { \
    tp_debug(("%s:%d\t: REG %d = %s : %x\n", __FUNCTION__, __LINE__, r, #val, \
        (u32)val)); \
    ARM_THM_MOVW(r, (val) & 0xffff); \
    ARM_THM_MOVT(r, ((val)>>16) & 0xffff); \
} while(0)

#define ARM_THM_CALL(addr) do { \
    ARM_THM_REG_SET(R4, (u32)(addr) | 0x1); \
    ARM_THM_BLX(R4); \
} while(0)

#define ARM_THM_ADD_SUB_SP(op, imm) OP16(ARM_THM_ADD_SUB_SP_VAL(op, imm))
#define ARM_THM_ADD_SP(imm) ARM_THM_ADD_SUB_SP(0, imm)
#define ARM_THM_SUB_SP(imm) ARM_THM_ADD_SUB_SP(1, imm)

#define ARM_THM_ADD_IMM(ld, imm) OP16(ARM_THM_ADD_IMM_VAL(ld, imm))

#define ARM_THM_CALL_PUSH_RET(func) do { \
    ARM_THM_CALL(func); \
    ARM_THM_PUSH(1<<R0); \
} while(0)

#define ARM_THM_STACK_ALLOC(reg, sz) do { \
    ARM_THM_SUB_SP(sz); \
    ARM_THM_MOV_REG(reg, SP); \
} while(0)

static u16 *code_block_alloc(u16 *cur)
{
    u16 *ret;

    total_blocks++;

    ret = mem_cache_alloc(js_compiler_mem_cache);
    *ret = 0;
    /* Store offset to new code block in first two bytes of old code block */
    /* XXX: assuming 2 bytes is enough */
    if (cur)
    {
        cur--;
        *cur = ret - cur;
    }

    return ret + 1;
}

/* Return pointer to next code block */
static u16 *code_block_free(u16 *block)
{
    int offset;

    block--;
    offset = (int)(s16)*block;
    mem_cache_free(js_compiler_mem_cache, block);
    return offset ? block + offset + 1 : NULL;
}

static void code_block_chain(void)
{
    u16 *cur_buf = op_buf, *next_buf;
    int delta;

    next_buf = code_block_alloc(cur_buf);

    /* XXX: make sure offset does not exceed available address space */

    /* B instruction offset is instruction_address + 4 + offset * 2
     * So if our pointers are u16 pointers, we need to subtract 2 to compensate
     * for the + 4
     */
    delta = next_buf - (cur_buf + op_buf_index) - 2;

    _op16(ARM_THM_B_VAL(delta));
    op_buf = next_buf;
    op_buf_index = 0;
}

static int compile_function_prologue(void)
{
    /* Store &ret (R0) in stack */
    ARM_THM_PUSH_POP(0, 1, (1<<R0)|(1<<R4)|(1<<R5));
    return 0;
}

static int compile_function_return(int rc)
{
    /* Fetch &ret from stack */
    ARM_THM_POP(1<<R0);
    /* Store return value in *ret */
    ARM_THM_STR(R1, R0, 0);
    ARM_THM_REG_SET(R0, rc);
    ARM_THM_PUSH_POP(1, 1, (1<<R4)|(1<<R5));
    return 0;
}

static int compile_num_new_int(int num)
{
    ARM_THM_REG_SET(R0, num);
    ARM_THM_CALL_PUSH_RET(num_new_int);
    return 0;
}

static int compile_num_new(tnum_t num)
{
    /* XXX: Support FP */
    if (NUMERIC_IS_FP(num))
       return -1;

    return compile_num_new_int(NUMERIC_INT(num));
}

static int compile_call_tstr_dup(tstr_t str)
{
    u32 *s = (u32 *)&str;

    ARM_THM_STACK_ALLOC(R0, 2);
    ARM_THM_REG_SET(R1, s[0]);
    ARM_THM_REG_SET(R2, s[1]);

    ARM_THM_CALL(tstr_dup);

    /* Fetch the dupped tstr from the stack */
    ARM_THM_POP(1<<R0);
    ARM_THM_POP(1<<R1);
    return 0;
}

static int compile_call_obj_do_op(token_type_t tok)
{
    ARM_THM_REG_SET(R0, tok);
    ARM_THM_POP(1<<R2);
    ARM_THM_POP(1<<R1);
    ARM_THM_CALL_PUSH_RET(obj_do_op);
    return 0;
}

static int compile_string_new(tstr_t str)
{
    /* tstr_dup the value so it can be used more than once */
    if (compile_call_tstr_dup(str))
        return -1;

    ARM_THM_CALL_PUSH_RET(string_new);
    return 0;
}

static int compile_expression(scan_t *scan);
static int compile_functions(scan_t *scan);

static int compile_atom(scan_t *scan)
{
    token_type_t tok = CUR_TOK(scan);

    switch (tok)
    {
    case TOK_NOT:
    case TOK_TILDE:
    case TOK_PLUS:
    case TOK_MINUS:
        js_scan_next_token(scan);

        ARM_THM_REG_SET(R1, (u32)ZERO);
        ARM_THM_PUSH(1<<R1);

        if (compile_functions(scan))
            return -1;

        if (compile_call_obj_do_op(tok))
            return -1;
        break;
    case TOK_NUM:
        {
            tnum_t num;

            if (js_scan_get_num(scan, &num))
                return -1;

            if (compile_num_new(num))
                return -1;
        }
        break;
    case TOK_CONSTANT:
        compile_num_new_int(js_scan_get_constant(scan));
        break;
    case TOK_STRING:
        {
            tstr_t str;

            if (js_scan_get_string(scan, &str))
                return -1;

            if (compile_string_new(str))
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

            if (compile_string_new(id))
            {
                tstr_free(&id);
                return -1;
            }

            /* XXX: memory leak since we don't provide means to free str */
        }
        break;
    case TOK_OPEN_PAREN:
        js_scan_next_token(scan);

        if (compile_expression(scan))
            return -1;

        if (_js_scan_match(scan, TOK_CLOSE_PAREN))
            return -1;

        break;
    default:
        return -1;
    }

    return 0;
}

static obj_t *get_property_helper(obj_t *property, obj_t ***lval)
{
    obj_t *o;
    tstr_t prop_name;

    prop_name = to_string(property)->value;

    o = obj_get_property(lval, cur_env, &prop_name);

    /* XXX: 'property' should be stored when assignement is implemented */
    obj_put(property);
    return o ? o : UNDEF;
}

static int compile_member(scan_t *scan)
{
    token_type_t tok = CUR_TOK(scan);

    if (compile_atom(scan))
        return -1;

    if (tok == TOK_ID)
    {
        ARM_THM_POP(1<<R0); /* compile_atom() return value */
        ARM_THM_MOV_REG(R1, R5); /* lval pointer */
        ARM_THM_CALL_PUSH_RET(get_property_helper);
    }

    return 0;
}

static obj_t *function_call_helper(int argc, obj_t *argv[])
{
    extern obj_t *global_env;
    obj_t *ret = UNDEF;

    function_call(&ret, global_env, argc, argv);

    return ret;
}

static int compile_function_call(scan_t *scan)
{
    int argc = 1;
    scan_t *start_args;

    js_scan_match(scan, TOK_OPEN_PAREN);

    /* Count argc */
    start_args = js_scan_save(scan);
    while (CUR_TOK(scan) != TOK_CLOSE_PAREN)
    {
        skip_expression(scan);
        if (CUR_TOK(scan) == TOK_COMMA)
            js_scan_next_token(scan);
        argc++;
    }

    js_scan_restore(scan, start_args);
    js_scan_free(start_args);

    /* keep function pointer */
    ARM_THM_POP(1<<R0);
    /* Allocate argv */
    ARM_THM_SUB_SP(argc - 1);
    /* Store argv[0] */
    ARM_THM_PUSH(1<<R0);

    if (CUR_TOK(scan) != TOK_CLOSE_PAREN)
    {
        int n = 0;

        if (compile_expression(scan))
            return -1;

        ARM_THM_ADD_SP(n + 2);
        ARM_THM_PUSH(1<<R1);
        ARM_THM_SUB_SP(n + 1);
        n++;
        while (CUR_TOK(scan) == TOK_COMMA)
        {
            js_scan_next_token(scan);
            if (compile_expression(scan))
                return -1;

            ARM_THM_ADD_SP(n + 2);
            ARM_THM_PUSH(1<<R1);
            ARM_THM_SUB_SP(n + 1);
            n++;
        }
    }
    if (CUR_TOK(scan) != TOK_CLOSE_PAREN)
        return -1;

    js_scan_match(scan, TOK_CLOSE_PAREN);

    ARM_THM_REG_SET(R0, argc);
    ARM_THM_MOV_REG(R1, SP); /* argv */
    ARM_THM_CALL(function_call_helper);
    ARM_THM_ADD_SP(argc); /* Unwind stack argv space */
    ARM_THM_PUSH(1<<R0); /* Store return value */
    return 0;
}

static int compile_functions(scan_t *scan)
{
    if (compile_member(scan))
        return -1;

    while (CUR_TOK(scan) == TOK_OPEN_PAREN)
    {
        if (compile_function_call(scan))
            return -1;
    }
    return 0;
}

#define GEN_COMPL(name, condition, lower) \
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
        if (compile_call_obj_do_op(tok)) \
            return -1; \
        tok = CUR_TOK(scan); \
    } \
    return 0; \
}

GEN_COMPL(compile_factor, (tok == TOK_DIV || tok == TOK_MULT || tok == TOK_MOD),
    compile_functions)
GEN_COMPL(compile_term, (tok == TOK_PLUS || tok == TOK_MINUS), compile_factor)
GEN_COMPL(compile_shift, (tok == TOK_SHL || tok == TOK_SHR || tok == TOK_SHRZ),
    compile_term)
GEN_COMPL(compile_related,
    (tok == TOK_IN || tok == TOK_GR || tok == TOK_GE || tok == TOK_LT ||
    tok == TOK_LE), compile_shift)
GEN_COMPL(compile_equalized,
    ((tok & ~STRICT) == TOK_IS_EQ || (tok & ~STRICT) == TOK_NOT_EQ),
    compile_related)
GEN_COMPL(compile_anded, (tok == TOK_AND), compile_equalized)
GEN_COMPL(compile_xored, (tok == TOK_XOR), compile_anded)
GEN_COMPL(compile_ored, (tok == TOK_OR), compile_xored)

static void assignment_helper(obj_t **ret, obj_t *val, obj_t *orig_val,
    obj_t **lval)
{
    obj_put(orig_val); /* Relese stored reference */
    *ret = val;
    *lval = obj_get(val);
    obj_put(orig_val); /* Release the reference we got */
}

static int compile_expression(scan_t *scan)
{
    ARM_THM_PUSH(1<<R5);
    ARM_THM_REG_SET(R5, (u32)UNDEF);
    ARM_THM_PUSH(1<<R5);
    ARM_THM_MOV_REG(R5, SP);

    if (compile_ored(scan))
        return -1;

    if (CUR_TOK(scan) == TOK_EQ)
    {
        js_scan_next_token(scan);
        if (compile_expression(scan))
            return -1;

        ARM_THM_POP(1<<R2); /* compile_ored() return value */
        ARM_THM_POP(1<<R3); /* Fetch lval */
        ARM_THM_STACK_ALLOC(R0, 1);
        ARM_THM_CALL(assignment_helper);
        ARM_THM_POP(1<<R1); /* Returned value */
    }
    else
    {
        ARM_THM_POP(1<<R1); /* compile_ored() return value */
        ARM_THM_POP(1<<R5); /* Discard of lval */
    }

    ARM_THM_POP(1<<R5);
    return 0;
}

static int compile_statement(scan_t *scan)
{
    switch (CUR_TOK(scan))
    {
    case TOK_END_STATEMENT:
        js_scan_next_token(scan);
        break;
    case TOK_RETURN:
        js_scan_match(scan, TOK_RETURN);
        if (CUR_TOK(scan) != TOK_END_STATEMENT)
        {
            if (compile_expression(scan))
                return -1;
        }
        compile_function_return(COMPLETION_RETURN);
        return 0;
    default:
        if (compile_expression(scan))
            return -1;

        if (_js_scan_match(scan, TOK_END_STATEMENT))
            return -1;

        break;
    }
    return 0;
}

static int compile_statement_list(scan_t *scan)
{
    while (!is_statement_list_terminator(CUR_TOK(scan)))
    {
        if (compile_statement(scan))
            return -1;
    }
    return 0;
}

static int _call_compiled_function(obj_t **ret, function_t *f)
{
    int (*compiled_func)(obj_t **ret);

    /* '1' in LSB denotes thumb function call */
    compiled_func = (int (*)(obj_t **))((u8 *)f->code + 1);
    return compiled_func(ret);
}

static int call_compiled_function(obj_t **ret, obj_t *this_obj, int argc,
    obj_t *argv[])
{
    return js_eval_wrap_function_execution(ret, this_obj, argc, argv,
        _call_compiled_function);
}

static void compiled_function_code_free(void *code)
{
    u16 *buffer = code;

    while ((buffer = code_block_free(buffer)));
}

static int compile_function(function_t *f)
{
    scan_t *code_copy;
    void *buffer;
    int rc;

    total_ops = 0;
    total_blocks = 0;
    op_buf_index = 0;

    op_buf = buffer = code_block_alloc(NULL);

    code_copy = js_scan_save(f->code);

    /* Skip opening bracket */
    js_scan_match(code_copy, TOK_OPEN_SCOPE);

    if ((rc = compile_function_prologue()))
        goto Error;

    if ((rc = compile_statement_list(code_copy)))
        goto Error;

    if ((rc = compile_function_return(COMPLETION_NORNAL)))
        goto Error;

    js_scan_free(code_copy);

    /* Destroy original code */
    f->code_free_cb(f->code);
    /* Place our new compiled code, call function and destructor */
    f->code = buffer;
    f->code_free_cb = compiled_function_code_free;
    f->call = call_compiled_function;
    tp_out(("Compilation status: Success\n"));
    tp_out(("ops %d, bytes %d, code blocks %d\n", total_ops,
        total_ops * sizeof(u16), total_blocks));
    return 0;

Error:
    js_scan_free(code_copy);
    compiled_function_code_free(buffer);
    return rc;
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

    if (compile_function(f))
        return throw_exception(po, &S("Function compilation failed"));

    return 0;
}

void js_compiler_uninit(void)
{
    mem_cache_destroy(js_compiler_mem_cache);
}

void js_compiler_init(void)
{
    js_compiler_mem_cache = mem_cache_create(MEM_CACHE_ITEM_SIZE,
        "JS Compiler");
}
