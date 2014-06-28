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
#include "util/tstr.h"
#include "util/debug.h"
#include "js/js_eval.h"
#include "js/js_eval_common.h"
#include "js/js_scan.h"
#include "js/js_types.h"
#include "js/js_obj.h"
#include "js/js_compiler.h"
#include "js/js_utils.h"
#include "js/js_builtins.h"

#define Sexception_invalid_lvalue_in_assign \
    S("Exception: Invalid left-hand value in assignment")
#define Sexception_undefined \
    S("Exception: Requested object is undefined")

typedef struct {
    obj_t **dst;
    obj_t *parent;
    obj_t *field;
    obj_t *base;
} reference_t;

extern obj_t *global_env;

#define JS_EVAL_FLAG_SIGNALLED 0x00000001
static volatile u32 g_flags;

#define EXECUTION_STOPPED() (g_flags & JS_EVAL_FLAG_SIGNALLED)
#define EXECUTION_STOPPED_SET() do { \
    g_flags |= JS_EVAL_FLAG_SIGNALLED; \
    tp_out(("Execution Stopped\n")); \
} while(0)

#define EXECUTION_STOPPED_RESET() g_flags &= ~JS_EVAL_FLAG_SIGNALLED

obj_t *cur_env;
static obj_t *this = NULL;
static function_args_t cur_function_args;

static int eval_expression(obj_t **po, scan_t *scan);
static int eval_block(obj_t **ret, scan_t *scan);
static int eval_if(obj_t **ret, scan_t *scan);
static int eval_while(obj_t **ret, scan_t *scan);
static int eval_do_while(obj_t **ret, scan_t *scan);
static int eval_for(obj_t **ret, scan_t *scan);
static int skip_block(obj_t **ret, scan_t *scan);
static int eval_function(obj_t **ret, scan_t *scan, int stmnt);
static int eval_functions(obj_t **po, scan_t *scan, reference_t *ref);
static int eval_statement_list(obj_t **ret, scan_t *scan);
static int eval_switch(obj_t **ret, scan_t *scan);
static int eval_assert_is_function(obj_t **po, scan_t *scan);

static int parse_error(obj_t **po)
{
    return throw_exception(po, &S("Exception: Parse error"));
}

static void function_args_bind(obj_t *env, tstr_list_t *params, 
    int argc, obj_t *argv[])
{
    int i = 0;

    for (; params; params = params->next)
    {
        obj_t *o = UNDEF;

        if (argc)
        {
            o = argv[i++];
            argc--;
        }
        obj_set_property(env, params->str, o);
    }
}

void evaluated_function_code_free(void *code)
{
    js_scan_free(code);
}

int js_eval_wrap_function_execution(obj_t **ret, obj_t *this_obj, int argc, 
    obj_t *argv[], int (*call)(obj_t **ret, function_t *f))
{
    function_args_t args = { .argc = argc, .argv = argv}, saved_args;
    obj_t *saved_env;
    int rc;
    function_t *func = to_function(argv[0]);

    saved_env = cur_env;
    cur_env = env_new(func->scope);
    function_args_bind(cur_env, func->formal_params, argc, argv);

    if (this_obj)
        this = this_obj;

    saved_args = cur_function_args;
    cur_function_args = args;

    rc = call(ret, func);
    
    cur_function_args = saved_args;

    if (rc == COMPLETION_NORNAL)
    {
        obj_put(*ret);
        *ret = UNDEF;
    }
    obj_put(cur_env);
    cur_env = saved_env;
    return rc;
}

static int _call_evaluated_function(obj_t **ret, function_t *func)
{
    scan_t *s;
    int rc;

    /* Create a duplicate scan for function code so we don't 
     * change the original scanner.
     */
    s = js_scan_save(func->code);

    if (CUR_TOK(s) == TOK_OPEN_SCOPE)
        rc = eval_block(ret, s);
    else
        rc = eval_statement_list(ret, s);
    
    js_scan_free(s);
    return rc;
}

int call_evaluated_function(obj_t **ret, obj_t *this_obj, int argc,
    obj_t *argv[])
{
    return js_eval_wrap_function_execution(ret, this_obj, argc, argv,
        _call_evaluated_function);
}

static int eval_function_call(obj_t **po, scan_t *scan, reference_t *ref,
    int construct)
{
    function_args_t args, saved_args;
    int i, rc;
    obj_t *saved_this = this, *o_func;

    if ((rc = eval_assert_is_function(po, scan)))
        return rc;

    saved_args = cur_function_args;

    o_func = *po;
    *po = UNDEF;
    function_args_init(&args, o_func);

    /* Arguments are optional in constructors calls */
    if (!construct || CUR_TOK(scan) == TOK_OPEN_PAREN)
    {
        obj_t *o;

        js_scan_match(scan, TOK_OPEN_PAREN);
        if (CUR_TOK(scan) != TOK_CLOSE_PAREN)
        {
            if ((rc = eval_expression(&o, scan)))
            {
                *po = o;
                goto Exit;
            }

            function_args_add(&args, o);

            while (CUR_TOK(scan) == TOK_COMMA)
            {
                js_scan_next_token(scan);
                if ((rc = eval_expression(&o, scan)))
                {
                    *po = o;
                    goto Exit;
                }

                function_args_add(&args, o);
            }
        }
        if (_js_scan_match(scan, TOK_CLOSE_PAREN))
        {
            rc = parse_error(po);
            goto Exit;
        }
    }

    cur_function_args = args;
    if (construct)
        rc = function_call_construct(po, args.argc, args.argv);
    else
    {
        obj_t *this_obj;

        this_obj = ref->parent ? : global_env;
        rc = function_call(po, this_obj, args.argc, args.argv);
    }
    if (rc == COMPLETION_RETURN)
        rc = 0;

Exit:
    this = saved_this;
    cur_function_args = saved_args;
    for (i = 1; i < args.argc; i++)
        obj_put(args.argv[i]);
    function_args_uninit(&args);
    obj_put(o_func);
    return rc;
}

static int eval_var_single(obj_t **ret, scan_t *scan)
{
    tstr_t name;
    obj_t *o = UNDEF;
    int rc = 0;

    *ret = UNDEF;

    if (js_scan_get_identifier(scan, &name))
        return parse_error(ret);

    if (CUR_TOK(scan) == TOK_EQ)
    {
        js_scan_next_token(scan);
        if ((rc = eval_expression(&o, scan)))
        {
            *ret = o;
            tstr_free(&name);
            return rc;
        }
    }
    else
        o = UNDEF;

    obj_set_property(cur_env, name, o);
    obj_put(o);
    tstr_free(&name);
    return rc;
}

static int eval_var(obj_t **ret, scan_t *scan)
{
    int rc;

    js_scan_match(scan, TOK_VAR);

    if ((rc = eval_var_single(ret, scan)))
        return rc;

    while (CUR_TOK(scan) == TOK_COMMA)
    {
        js_scan_next_token(scan);
        if ((rc = eval_var_single(ret, scan)))
            return rc;
    }
    return rc;
}

static inline int is_assignment_tok(token_type_t tok)
{
    return tok == TOK_PLUS_EQ || tok == TOK_MINUS_EQ || tok == TOK_MULT_EQ || 
        tok == TOK_DIV_EQ || tok == TOK_AND_EQ || tok == TOK_OR_EQ ||
        tok == TOK_XOR_EQ || tok == TOK_MOD_EQ || tok == TOK_EQ ||
        tok == TOK_SHR_EQ || tok == TOK_SHL_EQ || tok == TOK_SHRZ_EQ;
}

static inline int is_member_tok(token_type_t tok)
{
    return tok == TOK_DOT || tok == TOK_OPEN_MEMBER;
}

int parse_function_param_list(tstr_list_t **params, scan_t *scan)
{
    tstr_t param;

    if (js_scan_get_identifier(scan, &param))
        return -1;

    tstr_list_add(params, &param);

    while (CUR_TOK(scan) == TOK_COMMA)
    {
        js_scan_next_token(scan);
        if (js_scan_get_identifier(scan, &param))
        {
            tstr_list_free(params);
            return -1;
        }

        tstr_list_add(params, &param);
    }
    return 0;
}

static void ref_invalidate(reference_t *ref)
{
    obj_put(ref->parent);
    ref->parent = NULL;
    obj_put(ref->field);
    ref->field = NULL;
    ref->dst = NULL;
    ref->base = NULL;
}

static inline int valid_lval(reference_t *ref)
{
    return ref->dst ? 1 : 0;
}

static int eval_function_definition(tstr_t *fname, obj_t **po, scan_t *scan)
{
    obj_t *o;
    tstr_list_t *params = NULL;
    scan_t *start, *end;

    if (_js_scan_match(scan, TOK_OPEN_PAREN))
        return parse_error(po);

    tstr_list_add(&params, fname);

    if (CUR_TOK(scan) == TOK_ID)
    {
        if (parse_function_param_list(&params, scan))
            goto ParseError;
    }

    if (_js_scan_match(scan, TOK_CLOSE_PAREN))
        goto ParseError;

    start = js_scan_save(scan);
    if (skip_block(po, scan))
    {
        js_scan_free(start);
        goto ParseError;
    }
        
    end = js_scan_save(scan);
    o = function_new(params, js_scan_slice(start, end),
        evaluated_function_code_free, cur_env, call_evaluated_function);
    js_scan_free(start);
    js_scan_free(end);
    *po = o;
    return 0;

ParseError:
    tstr_list_free(&params);
    return parse_error(po);
}

static int eval_property(obj_t **po, scan_t *scan, obj_t *o)
{
    tstr_t property;
    token_type_t tok = CUR_TOK(scan);
    int rc;
    
    switch (tok)
    {
    case TOK_PROTOTYPE:
        js_scan_next_token(scan);
        break;
    case TOK_ID:
        js_scan_get_identifier(scan, &property);
        break;
    case TOK_STRING:
        js_scan_get_string(scan, &property);
        break;
    case TOK_NUM:
        {
            tnum_t num;
            js_scan_get_num(scan, &num);

            property = tnum_to_tstr(&num);
        }
        break;
    default:
        return parse_error(po);
    }

    if (_js_scan_match(scan, TOK_COLON))
        return parse_error(po);

    if ((rc = eval_expression(po, scan)))
        return rc;

    switch (tok)
    {
    case TOK_PROTOTYPE:
        property = Sprototype;
    case TOK_ID:
    case TOK_STRING:
    case TOK_NUM:
        _obj_set_property(o, property, *po);
        break;
    }
    tstr_free(&property);
    return 0;
}

static int eval_object(obj_t **po, scan_t *scan)
{
    obj_t *o = object_new();
    int rc;

    js_scan_match(scan, TOK_OPEN_SCOPE);

    while (CUR_TOK(scan) != TOK_CLOSE_SCOPE)
    {
        if ((rc = eval_property(po, scan, o)))
        {
            obj_put(o);
            return rc;
        }

        if (CUR_TOK(scan) != TOK_COMMA)
            break;

        js_scan_next_token(scan);
    }

    if ((_js_scan_match(scan, TOK_CLOSE_SCOPE)))
    {
        obj_put(o);
        return parse_error(po);
    }
    *po = o;
    return 0;
}

static int eval_array(obj_t **po, scan_t *scan)
{
    obj_t *o = array_new(), *val;
    
    js_scan_match(scan, TOK_OPEN_MEMBER);
    if (CUR_TOK(scan) == TOK_CLOSE_MEMBER)
        goto Exit; /* Empty array */

    eval_expression(&val, scan);
    array_push(o, val);
    while (CUR_TOK(scan) == TOK_COMMA)
    {
        js_scan_next_token(scan);
        eval_expression(&val, scan);
        array_push(o, val);
    }

Exit:
    js_scan_match(scan, TOK_CLOSE_MEMBER);
    *po = o;
    return 0;
}

static int eval_atom(obj_t **po, scan_t *scan, obj_t *obj, reference_t *ref)
{
    token_type_t tok = CUR_TOK(scan);
    int rc = 0;

    switch (tok)
    {
    case TOK_OPEN_PAREN:
        js_scan_next_token(scan);
        if ((rc = eval_expression(po, scan)))
            return rc;
        if (_js_scan_match(scan, TOK_CLOSE_PAREN))
            return parse_error(po);
        break;
    case TOK_THIS:
        js_scan_next_token(scan);
        tp_assert(this);
        ref_invalidate(ref);
        *po = obj_get(this);
        break;
    case TOK_ARGUMENTS:
        js_scan_next_token(scan);
        if (!cur_function_args.argc)
            return throw_exception(po, &S("Exception: Not in function call"));

        ref_invalidate(ref);
        *po = arguments_new(&cur_function_args);
        break;
    case TOK_PROTOTYPE:
        js_scan_next_token(scan);
        tp_assert(obj);
        *po = obj_get_own_property(&ref->dst, obj, &Sprototype);
        if (!*po)
        {
            obj_put(ref->field);
            ref->field = string_new(Sprototype);
            ref->base = obj;
            ref->dst = NULL;
            *po = UNDEF;
        }
        break;
    case TOK_OPEN_SCOPE:
        rc = eval_object(po, scan);
        break;
    case TOK_OPEN_MEMBER:
        rc = eval_array(po, scan);
        break;
    case TOK_FUNCTION:
        rc = eval_function(po, scan, 0);
        break;
    case TOK_NOT:
    case TOK_TILDE:
    case TOK_PLUS:
    case TOK_MINUS:
        js_scan_next_token(scan);
        rc = eval_functions(po, scan, ref);
        *po = obj_do_op(tok, ZERO, *po);
        break;
    case TOK_NUM:
        {
            tnum_t num;

            if (js_scan_get_num(scan, &num))
                return parse_error(po);

            *po = num_new(num);
        }
        break;
    case TOK_TRUE:
    case TOK_FALSE:
        js_scan_next_token(scan);
        *po = tok == TOK_TRUE ? TRUE : FALSE;
        break;
    case TOK_NULL:
        js_scan_next_token(scan);
        *po = NULL_OBJ;
        break;
    case TOK_UNDEFINED:
        js_scan_next_token(scan);
        *po = UNDEF;
        break;
    case TOK_STRING:
        {
            tstr_t str;

            if (js_scan_get_string(scan, &str))
                return parse_error(po);

            *po = string_new(str);
        }
        break;
    case TOK_ID:
        {
            tstr_t id;

            if (js_scan_get_identifier(scan, &id))
                return parse_error(po);

            *po = string_new(id);
        }
        break;
    case TOK_CONSTANT:
        *po = num_new_int(js_scan_get_constant(scan));
        break;
    default:
        return parse_error(po);
    }

    return rc;
}

static obj_t *get_property(obj_t *obj, obj_t *property, reference_t *ref)
{
    obj_t *o;
    tstr_t prop_name = obj_get_str(property);

    if (obj)
        ref->base = obj;
    else
    {
        obj = cur_env;
        /* If an unknown identifier appears, it is considered a candidate
         * for the global environment 
         * XXX: on strict mode, an exception is thrown
         */
        ref->base = global_env;
    }

    o = obj_get_property(&ref->dst, obj, &prop_name);

    obj_put(ref->field);
    ref->field = property;
    tstr_free(&prop_name);
    return o ? o : UNDEF;
}

static inline void ref_set_parent(reference_t *ref, obj_t *parent)
{
    if (!parent)
        return;

    obj_put(ref->parent);
    ref->parent = parent;
}

static int eval_member(obj_t **po, scan_t *scan, obj_t *o, reference_t *ref)
{
    obj_t *parent = obj_get(o);
    int rc = 0;

    /* XXX: rewrite */
    if (!o)
    {
        token_type_t tok = CUR_TOK(scan);

        rc = eval_atom(po, scan, NULL, ref);
        o = *po;
        if (tok == TOK_ID)
            o = *po = get_property(NULL, *po, ref);
    }

    while (is_member_tok(CUR_TOK(scan)))
    {
        /* XXX: only extensible objects can be used here */
        if (o == UNDEF)
        {
            ref_invalidate(ref);
            js_scan_trace(scan);
            return throw_exception(po, 
                &S("Exception: Can't access property of undefined"));
        }

        ref_set_parent(ref, parent);
        parent = o;
        if (CUR_TOK(scan) == TOK_DOT)
        {
            token_type_t tok;

            js_scan_next_token(scan);
            tok = CUR_TOK(scan);
            tp_assert(tok == TOK_ID || tok == TOK_PROTOTYPE);
            rc = eval_atom(po, scan, o, ref);
            o = *po;
            if (tok == TOK_ID)
                o = get_property(parent, o, ref);
        }
        else if (CUR_TOK(scan) == TOK_OPEN_MEMBER)
        {
            js_scan_next_token(scan);
            eval_expression(&o, scan);
            js_scan_match(scan, TOK_CLOSE_MEMBER);
            o = get_property(parent, o, ref);
        }
    }
    ref_set_parent(ref, parent);
    *po = o;
    return rc;
}

static int eval_assert_is_function(obj_t **po, scan_t *scan)
{
    obj_t *o_func = *po;
    tstr_t *error = NULL;

    if (o_func == UNDEF)
        error = &S("Exception: Object is undefined, not a function");
    else if (!is_function(o_func))
        error = &S("Exception: Object is not a function");

    if (error)
    {
        js_scan_trace(scan);
        /* Not a valid function. Throw exception */
        return throw_exception(po, error);
    }

    return 0;
}

static int eval_new(obj_t **po, scan_t *scan, reference_t *ref)
{
    int rc, is_new = 0;

    if (CUR_TOK(scan) == TOK_NEW)
    {
        is_new = 1;
        js_scan_next_token(scan);
    }

    if ((rc = eval_member(po, scan, NULL, ref)))
        return rc;

    if (is_new)
    {
        rc = eval_function_call(po, scan, ref, 1);
        /* constructed objects are not valid lvalues */
        ref_invalidate(ref);
    }

    return rc;
}

static int eval_functions(obj_t **po, scan_t *scan, reference_t *ref)
{
    int rc;

    rc = eval_new(po, scan, ref);

    while (!rc && CUR_TOK(scan) == TOK_OPEN_PAREN)
    {
        if ((rc = eval_function_call(po, scan, ref, 0)))
            return rc;

        /* Function calls do not return references */
        ref_invalidate(ref);
        if (*po && is_member_tok(CUR_TOK(scan)))
            rc = eval_member(po, scan, *po, ref);
    }

    return rc;
}

static int eval_postfix(obj_t **po, scan_t *scan, reference_t *ref)
{
    obj_t *o = UNDEF;
    token_type_t tok;
    int rc = 0;

    if ((rc = eval_functions(&o, scan, ref)))
        goto Exit;

    tok = CUR_TOK(scan);
    if (tok != TOK_PLUS_PLUS && tok != TOK_MINUS_MINUS)
        goto Exit;

    if (!valid_lval(ref))
    {
        tstr_t property;

        if (!o || o == UNDEF || !ref->base)
        {
            obj_put(o);
            return throw_exception(po, &Sexception_invalid_lvalue_in_assign);
        }

        property = obj_get_str(ref->field);
        _obj_set_property(ref->base, property, obj_do_op(tok, obj_get(o), 
            ZERO));
        tstr_free(&property);
    }
    else
    {
        /* its ok to release o in obj_do_op as we are taking the stored
         * reference and overriding it with the new_value.
         */
        *ref->dst = obj_do_op(tok, o, ZERO);
    }

    js_scan_next_token(scan);

    /* Invalidate returned reference as we are no longer a valid 
     * lvalue 
     */
    ref_invalidate(ref);

Exit:
    *po = o;
    return rc;
}

static int eval_pre_fix(obj_t **po, scan_t *scan, reference_t *ref)
{
    token_type_t tok = CUR_TOK(scan);
    obj_t *old_object, *o = UNDEF;
    int rc = 0, is_valid_lval;

    if (tok != TOK_PLUS_PLUS && tok != TOK_MINUS_MINUS)
        return eval_postfix(po, scan, ref);

    js_scan_next_token(scan);
    if ((rc = eval_postfix(&o, scan, ref)))
        goto Exit;

    is_valid_lval = valid_lval(ref);
    if (!is_valid_lval && (!o || o == UNDEF || !ref->base))
    {
        obj_put(o);
        return throw_exception(po, &Sexception_invalid_lvalue_in_assign);
    }

    /* Calculate new value */
    old_object = o;
    o = obj_do_op(tok, old_object, ZERO);

    if (!is_valid_lval)
    {
        tstr_t property = obj_get_str(ref->field);
        obj_set_property(ref->base, property, o);
        tstr_free(&property);
    }
    else
    {
        *ref->dst = obj_get(o);
        /* Release stored copy of old value */
        obj_put(old_object);

    }

    /* We are no longer a valid lvalue */
    ref_invalidate(ref);

Exit:
    *po = o;
    return rc;
}

#define GEN_EVAL(name, condition, skip_condition, lower) \
static int name(obj_t **po, scan_t *scan, reference_t *ref) \
{ \
    int rc = 0; \
    token_type_t tok; \
    if ((rc = lower(po, scan, ref))) \
        return rc; \
    tok = CUR_TOK(scan); \
    if (!condition) \
        return rc; \
    do \
    { \
        obj_t *o = UNDEF; \
        ref_invalidate(ref); \
        js_scan_next_token(scan); \
        if (skip_condition) \
        { \
            skip_expression(scan); \
            return rc; \
        } \
        if ((rc = lower(&o, scan, ref))) \
        { \
            obj_put(*po); \
            *po = o; \
            return rc; \
        } \
        *po = obj_do_op(tok, *po, o); \
        tok = CUR_TOK(scan); \
    } while (condition); \
    ref_invalidate(ref); \
    return rc; \
}

GEN_EVAL(eval_factor_expression,
    (tok == TOK_DIV || tok == TOK_MULT || tok == TOK_MOD), 0, eval_pre_fix)
GEN_EVAL(eval_term_expression,
    (tok == TOK_PLUS || tok == TOK_MINUS), 0, eval_factor_expression)
GEN_EVAL(eval_shifted_expression,
    (tok == TOK_SHL || tok == TOK_SHR || tok == TOK_SHRZ), 0,
    eval_term_expression)
GEN_EVAL(eval_related_expression,
    (tok == TOK_IN || tok == TOK_GR || tok == TOK_GE || tok == TOK_LT || 
    tok == TOK_LE), 0, eval_shifted_expression)
GEN_EVAL(eval_equalized_expression, 
    ((tok & ~STRICT) == TOK_IS_EQ || (tok & ~STRICT) == TOK_NOT_EQ), 0,
    eval_related_expression)
GEN_EVAL(eval_anded_expression, (tok == TOK_AND), 0, eval_equalized_expression)
GEN_EVAL(eval_xored_expression, (tok == TOK_XOR), 0, eval_anded_expression)
GEN_EVAL(eval_ored_expression, (tok == TOK_OR), 0, eval_xored_expression)
GEN_EVAL(eval_log_anded_expression, (tok == TOK_LOG_AND), (!obj_true(*po)),
    eval_ored_expression)
GEN_EVAL(eval_log_ored_expression, (tok == TOK_LOG_OR), (obj_true(*po)),
    eval_log_anded_expression)

static int eval_ternary_expression(obj_t **po, scan_t *scan, reference_t *ref)
{
    reference_t new_ref = {};
    int rc, condition;

    rc = eval_log_ored_expression(po, scan, ref);

    if (rc || CUR_TOK(scan) != TOK_QUESTION)
        return rc;

    condition = obj_true(*po);
    obj_put(*po);

    js_scan_match(scan, TOK_QUESTION);
    if (condition)
    {
        rc = eval_ternary_expression(po, scan, &new_ref);
        js_scan_match(scan, TOK_COLON);
        skip_expression(scan);
    }
    else
    {
        skip_expression(scan);
        js_scan_match(scan, TOK_COLON);
        rc = eval_ternary_expression(po, scan, &new_ref);
    }
    obj_put(new_ref.field);
    return rc;
}

static int eval_assignment(obj_t **po, scan_t *scan, reference_t *ref)
{
    token_type_t tok = CUR_TOK(scan);
    obj_t *old_object, *o, **dst;
    int rc = 0;
    
    old_object = *po;
    
    if (!valid_lval(ref))
    {
        if ((!old_object || old_object == UNDEF) && tok != TOK_EQ)
            return throw_exception(po, &Sexception_undefined);

        if (!ref->base)
            return throw_exception(po, &Sexception_invalid_lvalue_in_assign);
    }

    js_scan_next_token(scan);

    /* Get new value */
    if ((rc = eval_expression(po, scan)))
        return rc;
    
    if (valid_lval(ref))
    {
        dst = ref->dst;
        old_object = *dst;
        switch (tok)
        {
        case TOK_EQ:
            /* Release the previously stored reference */
            obj_put(old_object);
            *dst = obj_get(*po);
            /* Release the reference we got for old value as no one needs it */
            obj_put(old_object);
            break;
        default:
            /* Keep old reference as we are returning it */
            o = old_object;
            *dst = obj_do_op(tok & ~EQ, old_object, *po);
            *po = o;
            break;
        }
    }
    else
    {
        tstr_t property = obj_get_str(ref->field);
        switch (tok)
        {
        case TOK_EQ:
            obj_set_property(ref->base, property, *po);
            /* Release the reference we got for old value as no one needs it */
            obj_put(old_object);
            break;
        default:
            /* Keep old reference as we are returning it */
            o = obj_get(old_object);
            _obj_set_property(ref->base, property, 
                obj_do_op(tok & ~EQ, old_object, *po));
            *po = o;
            break;
        }
        tstr_free(&property);
    }

    return 0;
}

static inline int eval_expression_ref(obj_t **po, scan_t *scan, 
    reference_t *ref)
{
    return eval_ternary_expression(po, scan, ref); 
}

static int eval_expression(obj_t **po, scan_t *scan)
{
    reference_t ref = {};
    int rc;

    if ((rc = eval_expression_ref(po, scan, &ref)))
        goto Exit;

    if (is_assignment_tok(CUR_TOK(scan)))
    {
        if ((rc = eval_assignment(po, scan, &ref)))
            goto Exit;
    }
    
Exit:
    ref_invalidate(&ref);
    return rc;
}

static int eval_return(obj_t **ret, scan_t *scan)
{
    int rc = 0;

    js_scan_match(scan, TOK_RETURN);
    if (CUR_TOK(scan) != TOK_END_STATEMENT)
        rc = eval_expression(ret, scan);
    return rc ? rc : COMPLETION_RETURN;
}

static int eval_continue(obj_t **ret, scan_t *scan)
{
    js_scan_match(scan, TOK_CONTINUE);
    return COMPLETION_CONTINUE;
}

static int eval_break(obj_t **ret, scan_t *scan)
{
    js_scan_match(scan, TOK_BREAK);
    return COMPLETION_BREAK;
}

static int eval_throw(obj_t **ret, scan_t *scan)
{
    js_scan_match(scan, TOK_THROW);
    /* XXX: nested exceptions? */
    eval_expression(ret, scan);
    js_scan_match(scan, TOK_END_STATEMENT);
    return COMPLETION_THROW;
}

static int do_block(obj_t **ret, scan_t *scan)
{
    int rc;
    scan_t *start, *end;

    start = js_scan_save(scan);
    if ((rc = skip_block(ret, scan)))
    {
        js_scan_free(start);
        return rc;
    }
    end = js_scan_save(scan);

    js_scan_restore(scan, start);
    rc = eval_block(ret, scan);
    js_scan_restore(scan, end);

    js_scan_free(start);
    js_scan_free(end);
    return rc;
}

static int eval_try(obj_t **ret, scan_t *scan)
{
    int rc;
    tstr_t id;

    js_scan_match(scan, TOK_TRY);

    rc = do_block(ret, scan);

    if (CUR_TOK(scan) == TOK_CATCH)
    {
        js_scan_next_token(scan);
        js_scan_match(scan, TOK_OPEN_PAREN);
        tp_assert(CUR_TOK(scan) == TOK_ID);
        js_scan_get_identifier(scan, &id);
        js_scan_match(scan, TOK_CLOSE_PAREN);
        if (rc == COMPLETION_THROW)
        {
            obj_t *saved_env;

            saved_env = cur_env;
            cur_env = env_new(cur_env);

            /* Bind the thrown value to the new env. Our reference will be 
             * put when the new env is put.
             */
            obj_set_property(cur_env, id, *ret);
            obj_put(*ret);
            *ret = UNDEF;

            /* XXX: nested exceptions? */
            do_block(ret, scan);

            obj_put(cur_env);
            cur_env = saved_env;
        }
        else
        {
            obj_t *o = UNDEF;

            skip_block(&o, scan);
            obj_put(o); /* Don't mind the error for now */
        }

        tstr_free(&id);
    }
    *ret = UNDEF;
    return 0;
}

static int eval_statement(obj_t **ret, scan_t *scan)
{
    int rc = 0;

    *ret = UNDEF;

    js_scan_set_trace_point(scan);

    switch (CUR_TOK(scan))
    {
    case TOK_END_STATEMENT:
        js_scan_next_token(scan);
        break;
    case TOK_OPEN_SCOPE:
        return eval_block(ret, scan);
    case TOK_IF:
        return eval_if(ret, scan);
    case TOK_WHILE:
        return eval_while(ret, scan);
    case TOK_DO:
        return eval_do_while(ret, scan);
    case TOK_FOR:
        return eval_for(ret, scan);
    case TOK_VAR:
        if ((rc = eval_var(ret, scan)))
            return rc;

        if (_js_scan_match(scan, TOK_END_STATEMENT))
            return parse_error(ret);
        break;
    case TOK_RETURN:
        return eval_return(ret, scan);
    case TOK_THROW:
        return eval_throw(ret, scan);
    case TOK_CONTINUE:
        return eval_continue(ret, scan);
    case TOK_BREAK:
        return eval_break(ret, scan);
    case TOK_TRY:
        return eval_try(ret, scan);
    case TOK_FUNCTION:
        return eval_function(ret, scan, 1);
    case TOK_SWITCH:
        return eval_switch(ret, scan);
    case TOK_CASE:
    case TOK_DEFAULT:
        /* Handled in eval_switch */
        break;
    default:
        if ((rc = eval_expression(ret, scan)))
            return rc;

        if (_js_scan_match(scan, TOK_END_STATEMENT))
            return parse_error(ret);
        break;
    }

    return rc;
}

static int skip_block(obj_t **ret, scan_t *scan)
{
    if (_js_scan_match(scan, TOK_OPEN_SCOPE))
        return parse_error(ret);

    while (CUR_TOK(scan) != TOK_CLOSE_SCOPE && CUR_TOK(scan) != TOK_EOF)
    {
        if (CUR_TOK(scan) == TOK_OPEN_SCOPE)
        {
            int rc;
          
            if ((rc = skip_block(ret, scan)))
                return rc;

            continue;
        }
        js_scan_next_token(scan);
    }
    if (_js_scan_match(scan, TOK_CLOSE_SCOPE))
        return parse_error(ret);

    return 0;
}

static void skip_for(scan_t *scan)
{
    js_scan_match(scan, TOK_FOR);
    js_scan_match(scan, TOK_OPEN_PAREN);
    skip_expression(scan);
    if (CUR_TOK(scan) == TOK_CLOSE_PAREN)
    {
        /* Probably for-in loop. finish up */
        goto Exit;
    }

    js_scan_match(scan, TOK_END_STATEMENT);
    skip_expression(scan);
    js_scan_match(scan, TOK_END_STATEMENT);
    skip_expression(scan);

Exit:
    js_scan_next_token(scan); /* ) */
}

/* XXX: support exception on statement skipping errors */
static void skip_statement(scan_t *scan)
{
    if (CUR_TOK(scan) == TOK_OPEN_SCOPE)
    {
        obj_t *o = UNDEF;

        skip_block(&o, scan);
        return;
    }

    if (CUR_TOK(scan) == TOK_FOR)
        skip_for(scan);

    while (CUR_TOK(scan) != TOK_END_STATEMENT && CUR_TOK(scan) != TOK_EOF)
        js_scan_next_token(scan);
    js_scan_match(scan, TOK_END_STATEMENT);
}

static void skip_statement_list(scan_t *scan)
{
    while (!is_statement_list_terminator(CUR_TOK(scan)))
        skip_statement(scan);
}

static int eval_condition(scan_t *scan)
{
    obj_t *value;
    int is_true;
    
    eval_expression(&value, scan);
    is_true = obj_true(value);
    obj_put(value);
    return is_true;
}

static int eval_parenthesized_expression(obj_t **ret, scan_t *scan)
{
    int rc;
    obj_t *exp = UNDEF;

    if (_js_scan_match(scan, TOK_OPEN_PAREN))
        return parse_error(ret);

    rc = eval_expression(&exp, scan);
    if (!rc && _js_scan_match(scan, TOK_CLOSE_PAREN))
    {
        obj_put(exp);
        return parse_error(ret);
    }

    *ret = exp;
    return rc;
}

static int eval_parenthesized_condition(obj_t **ret, int *condition, 
    int skip, scan_t *scan)
{
    if (_js_scan_match(scan, TOK_OPEN_PAREN))
        return parse_error(ret);

    if (skip)
    {
        *condition = 0;
        skip_expression(scan);
    }
    else
        *condition = eval_condition(scan);

    if (_js_scan_match(scan, TOK_CLOSE_PAREN))
        return parse_error(ret);

    return 0;
}

static int _eval_if(obj_t **ret, scan_t *scan, int skip, int *condition)
{
    int rc = 0;

    *ret = UNDEF;

    if ((rc = eval_parenthesized_condition(ret, condition, skip, scan)))
        return rc;

    if (*condition && !skip)
        rc = eval_statement(ret, scan);
    else
        skip_statement(scan);

    return rc;
}

static int eval_if(obj_t **ret, scan_t *scan)
{
    int skip = 0, rc;
    
    *ret = UNDEF;
   
    js_scan_match(scan, TOK_IF);

    if ((rc = _eval_if(ret, scan, 0, &skip)))
        return rc;

    while (CUR_TOK(scan) == TOK_ELSE)
    {
        js_scan_next_token(scan);
        if (CUR_TOK(scan) == TOK_IF)
        {
            int val = 0;
            js_scan_next_token(scan);
            if ((rc = _eval_if(ret, scan, skip, &val)))
                return rc;

            skip |= val;
            continue;
        }
        /* last else */
        if (!skip)
            rc = eval_statement(ret, scan);
        else
            skip_statement(scan);
        break;
    }
    return rc;
}

static int eval_while(obj_t **ret, scan_t *scan)
{
    scan_t *start;
    int rc = 0;
    
    js_scan_match(scan, TOK_WHILE);
    start = js_scan_save(scan);
    while (1)
    {
        int next = 0;

        if ((rc = eval_parenthesized_condition(ret, &next, 
            rc == COMPLETION_BREAK, scan)))
        {
            return rc;
        }

        if (!next || EXECUTION_STOPPED())
        {
            skip_statement(scan);
            break;
        }
        rc = eval_statement(ret, scan);
        if (rc == COMPLETION_RETURN || rc == COMPLETION_THROW)
        {
            js_scan_free(start);
            return rc;
        }

        obj_put(*ret);
        js_scan_restore(scan, start);
    }
    *ret = UNDEF;
    js_scan_free(start);
    return rc;
}

static int eval_do_while(obj_t **ret, scan_t *scan)
{
    scan_t *start, *end;
    int rc, next = 0;
    
    js_scan_match(scan, TOK_DO);
    start = js_scan_save(scan);
    skip_statement(scan);
    js_scan_match(scan, TOK_WHILE);
    js_scan_match(scan, TOK_OPEN_PAREN);
    skip_expression(scan);
    js_scan_match(scan, TOK_CLOSE_PAREN);
    end = js_scan_save(scan);

    do
    {
        js_scan_restore(scan, start);
        rc = eval_statement(ret, scan);
        if (rc == COMPLETION_RETURN || rc == COMPLETION_THROW)
            break;

        obj_put(*ret);
        *ret = UNDEF;

        if (rc == COMPLETION_BREAK || rc == COMPLETION_CONTINUE ||
	    EXECUTION_STOPPED())
	{
            break;
	}

        js_scan_match(scan, TOK_WHILE);

        if ((rc = eval_parenthesized_condition(ret, &next, 0, scan)))
            next = 0;

    } while (next);

    js_scan_restore(scan, end);
    js_scan_free(start);
    js_scan_free(end);
    return rc;
}

/* XXX: rather naive "switch" implementation. 
 * 1. Very basic validity checks
 * 2. No support for
 *    case A:
 *    default:
 *    case B:
 */

static int eval_case(obj_t **ret, int *found_match, obj_t *match, scan_t *scan)
{
    obj_t *item = UNDEF;
    int rc = 0;

    switch (CUR_TOK(scan))
    {
    case TOK_CASE:
        js_scan_next_token(scan);
        if (*found_match)
            skip_expression(scan);
        else
        {
            if ((rc = eval_expression(&item, scan)))
            {
                *ret = item;
                return rc;
            }
            *found_match = obj_eq(item, match);
            obj_put(item);
        }
        break;
    case TOK_DEFAULT:
        js_scan_next_token(scan);
        *found_match = 1;
        break;
    default:
        return parse_error(ret);
    }

    if (_js_scan_match(scan, TOK_COLON))
        return parse_error(ret);

    return 0;
}

static int eval_switch(obj_t **ret, scan_t *scan)
{
    scan_t *start;
    obj_t *match = UNDEF;
    int rc = 0, found_match = 0;
    
    js_scan_match(scan, TOK_SWITCH);

    if ((rc = eval_parenthesized_expression(&match, scan)))
    {
        *ret = match;
        return rc;
    }

    start = js_scan_save(scan);

    if (_js_scan_match(scan, TOK_OPEN_SCOPE))
        goto ParseError;

    while (CUR_TOK(scan) != TOK_CLOSE_SCOPE)
    {
        if ((rc = eval_case(ret, &found_match, match, scan)))
            goto Exit;
        
        if (!found_match)
        {
            skip_statement_list(scan);
            continue;
        }

        if ((rc = eval_statement_list(ret, scan)))
        {
            obj_t *o = UNDEF;

            js_scan_restore(scan, start);
            /* We are already in error, don't mind the skip_block result */
            skip_block(&o, scan);
            obj_put(o);
            goto Exit;
        }
        obj_put(*ret);
        *ret = UNDEF;
    }

    if (_js_scan_match(scan, TOK_CLOSE_SCOPE))
        goto ParseError;

Exit:
    if (rc == COMPLETION_BREAK)
        rc = 0;
    if (!rc)
    {
        obj_put(*ret);
        *ret = UNDEF;
    }
    obj_put(match);
    js_scan_free(start);
    return rc;

ParseError:
    rc = parse_error(ret);
    /* Some sphagetti */
    goto Exit;
}

static int eval_for_in(obj_t **ret, scan_t *scan, scan_t *in_lhs, obj_t *rh_exp)
{
    scan_t *loop, *end;
    object_iter_t iter = {};
    int rc = 0;

    tp_info(("Iterating over %o\n", rh_exp));
    
    /* Body */
    loop = js_scan_save(scan);
    skip_statement(scan);
    end = js_scan_save(scan);

    /* Do the loop */
    object_iter_init(&iter, rh_exp);
    while (object_iter_next(&iter))
    {
        reference_t ref = {};
        obj_t *lhs = UNDEF, **dst;

        tp_info(("key %S\n", iter.key));
        js_scan_restore(scan, in_lhs);
        if ((rc = eval_expression_ref(&lhs, scan, &ref)))
        {
            *ret = lhs;
            goto Exit;
        }
        
        tp_info(("lhs %o\n", lhs));
        
        /* Replace lhs with current key */
        if (valid_lval(&ref))
        {
            dst = ref.dst;
            obj_put(*dst);
        }
        else
        {
            tstr_t field_tstr = obj_get_str(ref.field);
            dst = obj_var_create(ref.base, &field_tstr);
            tstr_free(&field_tstr);
        }

        *dst = string_new(tstr_dup(*iter.key));
        obj_put(lhs);
        ref_invalidate(&ref);

        /* Eval loop body */
        js_scan_restore(scan, loop);
        rc = eval_statement(ret, scan);
        if (rc == COMPLETION_RETURN || rc == COMPLETION_THROW)
            goto Exit;

        obj_put(*ret);
        *ret = UNDEF;

        if (rc == COMPLETION_BREAK || EXECUTION_STOPPED())
        {
            rc = 0;
            goto Exit;
        }
    }

Exit:
    object_iter_uninit(&iter);
    js_scan_restore(scan, end);
    js_scan_free(loop);
    js_scan_free(end);
    return rc;
}

static int parse_for_in(scan_t *scan, scan_t **lhs, obj_t **rh_exp)
{
    scan_t *last = NULL, *start = js_scan_save(scan);
    int in_found = 0, end_stmnt = 0;

    while (CUR_TOK(scan) != TOK_CLOSE_PAREN && CUR_TOK(scan) != TOK_EOF)
    {
        if ((end_stmnt = CUR_TOK(scan) == TOK_END_STATEMENT))
            break;
        
        if (CUR_TOK(scan) == TOK_OPEN_PAREN)
        {
            js_scan_match(scan, TOK_OPEN_PAREN);
            skip_expression(scan);
            js_scan_match(scan, TOK_CLOSE_PAREN);
            continue;
        }
        
        if (CUR_TOK(scan) == TOK_OPEN_MEMBER)
        {
            js_scan_match(scan, TOK_OPEN_MEMBER);
            skip_expression(scan);
            js_scan_match(scan, TOK_CLOSE_MEMBER);
            continue;
        }

        if ((in_found = CUR_TOK(scan) == TOK_IN))
        {
            *lhs = js_scan_slice(start, last);
            js_scan_match(scan, TOK_IN);
            eval_expression(rh_exp, scan);
            break;
        }

        js_scan_free(last);
        last = js_scan_save(scan);
        js_scan_next_token(scan);
    }
    if (!in_found || end_stmnt)
        js_scan_restore(scan, start);
    else
        js_scan_match(scan, TOK_CLOSE_PAREN);
    js_scan_free(start);
    js_scan_free(last);
    return in_found && !end_stmnt;
}

static int eval_for(obj_t **ret, scan_t *scan)
{
    scan_t *in_lhs, *loop, *cond, *repeated, *end;
    int next = 1, rc = 0;
    obj_t *rh_exp;
      
    *ret = UNDEF;

    js_scan_match(scan, TOK_FOR);

    /* Initializer */
    js_scan_match(scan, TOK_OPEN_PAREN);

    if (parse_for_in(scan, &in_lhs, &rh_exp))
    {
        int rc;

        tp_info(("For-in loop detected\n"));
        rc = eval_for_in(ret, scan, in_lhs, rh_exp);
        js_scan_free(in_lhs);
        obj_put(rh_exp);
        return rc;
    }

    /* XXX: This is not the prettiest way to do this... */
    if (CUR_TOK(scan) == TOK_VAR)
    {
        if ((rc = eval_var(ret, scan)))
            return rc;
    }
    else if (CUR_TOK(scan) == TOK_END_STATEMENT)
    {
        /* Nothing to do */
    }
    else
    {
        if ((rc = eval_expression(ret, scan)))
            return rc;

        obj_put(*ret);
    }
    js_scan_match(scan, TOK_END_STATEMENT);

    /* Condition */
    cond = js_scan_save(scan);
    next = CUR_TOK(scan) == TOK_END_STATEMENT ? 1 : eval_condition(scan);
    js_scan_match(scan, TOK_END_STATEMENT);

    /* Repeat */
    repeated = js_scan_save(scan);
    skip_expression(scan);
    js_scan_match(scan, TOK_CLOSE_PAREN);

    /* Body */
    loop = js_scan_save(scan);
    skip_statement(scan);
    end = js_scan_save(scan);

    /* Do the loop */
    while (next)
    {
        obj_t *o;

        js_scan_restore(scan, loop);
        rc = eval_statement(ret, scan);
        if (rc == COMPLETION_RETURN || rc == COMPLETION_THROW)
            goto Exit;

        obj_put(*ret);

        if (rc == COMPLETION_BREAK || EXECUTION_STOPPED())
        {
            js_scan_restore(scan, end);
            rc = 0;
            goto Exit;
        }

        js_scan_restore(scan, repeated);
        eval_expression(&o, scan);
        obj_put(o);
        js_scan_restore(scan, cond);
        if (!(next = eval_condition(scan)))
            js_scan_restore(scan, end);
    }

    *ret = UNDEF;

Exit:
    js_scan_free(cond);
    js_scan_free(repeated);
    js_scan_free(loop);
    js_scan_free(end);
    return rc;
}

static int eval_statement_list(obj_t **ret, scan_t *scan)
{
    int rc;

    *ret = UNDEF;

    while (!is_statement_list_terminator(CUR_TOK(scan)))
    {
        obj_t *o;

        if ((rc = eval_statement(&o, scan)))
        {
            obj_put(*ret);
            *ret = o;
            return rc;
        }
        /* We return the last VALUED statement's result */
        if (o != UNDEF)
        {
            obj_put(*ret);
            *ret = o;
        }
    }
    return 0;
}

static int eval_block(obj_t **ret, scan_t *scan)
{
    int rc;

    js_scan_match(scan, TOK_OPEN_SCOPE);

    if ((rc = eval_statement_list(ret, scan)))
        return rc;

    js_scan_match(scan, TOK_CLOSE_SCOPE);
    return 0;
}

static int eval_function(obj_t **ret, scan_t *scan, int stmnt)
{
    tstr_t func_name, *fname = &INTERNAL_S("__builtin_func__");
    int rc, have_func_name;

    js_scan_match(scan, TOK_FUNCTION);
    
    have_func_name = CUR_TOK(scan) == TOK_ID;

    if (have_func_name)
    {
        /* Have function name */

        js_scan_get_identifier(scan, &func_name);
        if (!stmnt)
        {
            /* Function expressions require binding to the function's lexical
             * scope.
             * fname will be owned by the function object.
             */
            fname = &func_name;
            TSTR_SET_INTERNAL(fname);
        }
    }
    
    if ((rc = eval_function_definition(fname, ret, scan)))
    {
        tstr_free(fname);
        return rc;
    }

    if (have_func_name && stmnt)
    {
        /* Statements require binding to environment */
        obj_set_property(cur_env, func_name, *ret);
        tstr_free(&func_name);
    }
    return 0;
}

int js_eval(obj_t **ret, tstr_t *code)
{
    scan_t *scan;
    int rc = 0;

    scan = js_scan_init(code);
    rc = eval_statement_list(ret, scan);
    js_scan_uninit(scan);

    EXECUTION_STOPPED_RESET();
    return rc;
}

int js_eval_module(obj_t **ret, tstr_t *code)
{
    obj_t *saved_env, *mod, *exports;
    int rc;

    saved_env = cur_env;
    cur_env = env_new(cur_env);

    mod = object_new();
    exports = object_new();
    obj_set_property(mod, S("exports"), exports);
    obj_set_property(cur_env, S("module"), mod);
    obj_put(mod);

    if ((rc = js_eval(ret, code)))
        goto Exit;

    obj_put(*ret);
    *ret = exports;

Exit:
    obj_put(cur_env);
    cur_env = saved_env;
    return rc;
}

int js_eval_obj(obj_t **ret, obj_t *obj)
{
    obj_t *saved_env;
    int rc;

    if (!is_string(obj))
    {
        *ret = obj;
        return 0;
    }

    saved_env = cur_env;
    cur_env = env_new(cur_env);

    rc = js_eval(ret, &to_string(obj)->value);

    obj_put(cur_env);
    cur_env = saved_env;
    return rc;
}

void js_eval_stop_execution(void)
{
    EXECUTION_STOPPED_SET();
}

void js_eval_uninit(void)
{
}

void js_eval_init(void)
{
    cur_env = global_env;
}
