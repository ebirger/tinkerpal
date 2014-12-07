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
#include "util/tp_types.h"
#include "util/tp_misc.h"
#include "util/tprintf.h"
#include "util/debug.h"
#include "mem/mem_cache.h"
#include "js/js_obj.h"
#include "js/js_types.h"
#include <math.h>
#include <float.h>

#define Slength INTERNAL_S("length")

struct var_t {
    var_t *next;
    tstr_t key;
    obj_t *obj;
};

typedef struct {
    void (*dump)(printer_t *printer, obj_t *o);
#ifdef CONFIG_OBJ_DOC
    int (*describe)(printer_t *printer, obj_t *o);
#endif
    void (*free)(obj_t *o);
    obj_t *(*do_op)(token_type_t op, obj_t *oa, obj_t *ob);
    int (*is_true)(obj_t *o);
    obj_t *(*cast)(obj_t *o, unsigned char class);
    void (*pre_var_create)(obj_t *o, const tstr_t *str);
    obj_t *(*get_own_property)(obj_t ***lval, obj_t *o, const tstr_t *str);
    int (*set_own_property)(obj_t *o, tstr_t str, obj_t *value);
} obj_class_t;

/* classes is defined at the bottom of this file.
 * It 'extern' instead of 'static' to avoid a mass of forward declarations
 */
extern const obj_class_t classes[];

static obj_t *class_prototypes[CLASS_LAST+1];
static mem_cache_t *obj_cache[CLASS_LAST];
static mem_cache_t *var_cache;

#define CLASS(obj) (&classes[OBJ_CLASS(obj)])
#define CLASS_PROTOTYPE(obj) (class_prototypes[OBJ_CLASS(obj)])

/* Global Objects */
obj_t undefind_obj = STATIC_OBJ(UNDEFINED_CLASS);
obj_t null_obj = STATIC_OBJ(NULL_CLASS);
num_t zero_obj = STATIC_NUM(0);
num_t nan_obj = STATIC_NUM(0xfeed); /* No meaning to value of the NaN object */
bool_t true_obj = { .obj = STATIC_OBJ(BOOL_CLASS), .is_true = 1 };
bool_t false_obj = { .obj = STATIC_OBJ(BOOL_CLASS), .is_true = 0 };

static obj_t *string_do_op(token_type_t op, obj_t *oa, obj_t *ob);
double num_fp_value(num_t *n);

/*** "vars" API ***/

static inline void var_key_free(tstr_t *key)
{
    tstr_free(key);
}

static inline int var_key_cmp(const tstr_t *keya, const tstr_t *keyb)
{
    return tstr_cmp(keya, keyb);
}

static inline int var_key_is_internal(tstr_t *key)
{
    return TSTR_IS_INTERNAL(key);
}

static void var_free(var_t *v)
{
    tp_debug(("freeing %p\n", v));
    obj_put(v->obj);
    var_key_free(&v->key);
    mem_cache_free(var_cache, v);
}

static void vars_free(var_t **vars)
{
    var_t *temp;

    while ((temp = *vars))
    {
        *vars = (*vars)->next;
        var_free(temp);
    }
}

static obj_t **var_get(var_t *vars, const tstr_t *key)
{
    var_t *iter;

    for (iter = vars; iter && var_key_cmp(&iter->key, key); iter = iter->next);
    if (!iter)
        return NULL;

    obj_get(iter->obj);
    return &iter->obj;
}

static obj_t **var_create(var_t **vars, tstr_t *key)
{
    var_t **iter;

    for (iter = vars; *iter && var_key_cmp(&(*iter)->key, key); 
        iter = &(*iter)->next);
    if (*iter)
    {
        /* Recycle */
        obj_put((*iter)->obj);
        (*iter)->obj = NULL;
    }
    else
    {
        /* Create new */
        *iter = mem_cache_alloc(var_cache);
        (*iter)->key = tstr_dup(*key);
        (*iter)->next = NULL;
        (*iter)->obj = NULL;
    }
    return &(*iter)->obj;
}

/*** Generic obj methods ***/

void _obj_put(obj_t *o)
{
    if (CLASS(o)->free)
        CLASS(o)->free(o);
    vars_free(&o->properties);
    tp_debug(("%s: freeing %p\n", __FUNCTION__, o));
    if (!(o->flags & OBJ_STATIC))
        mem_cache_free(obj_cache[OBJ_CLASS(o) - 1], o);
}

obj_t *obj_get_property(obj_t ***lval, obj_t *o, const tstr_t *property)
{
    obj_t **ref = NULL, *val = NULL, *proto;

    tp_debug(("Lookup %S in obj %p\n", &property, o));
    if ((val = obj_get_own_property(&ref, o, property)))
        goto Exit;

    proto = obj_get_own_property(NULL, o, &Sprototype);
    if (proto && proto != UNDEF)
    {
        val = obj_get_property(&ref, proto, property);
        obj_put(proto);
    }

    /* If this is an env obj, there is nothing more we can do.
     * Note: we do return references to prototype properties on
     * 'env' objects as they signify the 'outer' env (needed for closures)
     */
    if (is_env(o))
        goto Exit;

    /* If we couldn't find a better match, let's lookup the class_prototype,
     * Note that the class prototype of the "Object" class prototypes leads
     * to itself...
     */
    if (!val && CLASS_PROTOTYPE(o) && CLASS_PROTOTYPE(o) != o)
        val = obj_get_property(&ref, CLASS_PROTOTYPE(o), property);

    /* User is not allowed to change class or object prototypes */
    ref = NULL;

Exit:
    if (lval)
        *lval = ref;
    return val;
}

void obj_dump(printer_t *printer, obj_t *o)
{
    if (!o)
        return;

    CLASS(o)->dump(printer, o);
}

int obj_true(obj_t *o)
{
    tp_assert(CLASS(o)->is_true);

    return CLASS(o)->is_true(o);
}

void obj_describe(printer_t *printer, obj_t *o)
{
#ifndef CONFIG_OBJ_DOC
    tprintf(printer, "Object descriptions are available only when "
        "CONFIG_OBJ_DOC is enabled\n");
#else
    if (!o)
        return;

    if (!CLASS(o)->describe || CLASS(o)->describe(printer, o))
        tprintf(printer, "No description available\n");
#endif
}

obj_t *obj_get_own_property(obj_t ***lval, obj_t *o, const tstr_t *key)
{
    obj_t **ref;

    if (OBJ_IS_INT_VAL(o))
        return NULL;

    if ((ref = var_get(o->properties, key)))
    {
        if (lval)
            *lval = ref;
        return *ref;
    }

    if (CLASS(o)->get_own_property)
        return CLASS(o)->get_own_property(lval, o, key);

    return NULL;
}

obj_t *obj_cast(obj_t *o, unsigned char class)
{
    return CLASS(o)->cast(o, class);
}

obj_t *obj_has_property(obj_t *o, const tstr_t *property)
{
    obj_t *ret, *prop;

    prop = obj_get_property(NULL, o, property);
    ret = prop ? TRUE : FALSE;
    obj_put(prop);
    return ret;
}

obj_t *obj_do_in_op(obj_t *oa, obj_t *ob)
{
    obj_t *ret;
    tstr_t property;

    property = obj_get_str(oa);
    /* XXX: according to ECMAScript, ob must be an object, otherwise we need
     * to throw an exception, but we don't support exceptions in expressions
     * yet.
     */
    ret = obj_has_property(ob, &property);
    tstr_free(&property);
    return ret;
}

static inline void obj_to_num(obj_t **o)
{
    obj_t *tmp = *o;

    if (is_num(tmp))
        return;

    *o = obj_cast(tmp, NUM_CLASS);
    obj_put(tmp);
}

obj_t *obj_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    obj_t *ret;

    switch (op)
    {
    case TOK_NOT: ret = obj_true(ob) ? FALSE : TRUE; break;
    case TOK_LOG_AND: ret = obj_true(oa) && obj_true(ob) ? TRUE : FALSE; break;
    case TOK_LOG_OR: ret = obj_true(oa) || obj_true(ob) ? TRUE : FALSE; break;
    case TOK_IN:
        ret = obj_do_in_op(oa, ob);
        break;
    case TOK_MULT:
    case TOK_DIV:
    case TOK_MOD:
    case TOK_MINUS:
        obj_to_num(&oa);
        obj_to_num(&ob);
        goto do_op;
    case TOK_GR:
    case TOK_GE:
        if (oa == UNDEF)
        {
            ret = FALSE;
            break;
        }
        goto do_op;
    case TOK_LT:
    case TOK_LE:
        if (ob == UNDEF)
        {
            ret = FALSE;
            break;
        }
        goto do_op;
    case TOK_NOT_EQ_STRICT:
    case TOK_IS_EQ_STRICT:
        if (CLASS(oa) != CLASS(ob))
        {
            ret = op == TOK_NOT_EQ_STRICT ? TRUE : FALSE;
            break;
        }
        /* Fallthrough */
do_op:
    default:
        tp_assert(CLASS(oa)->do_op);
        ret = CLASS(oa)->do_op(op & ~STRICT, oa, ob);
    }

    obj_put(oa);
    obj_put(ob);
    return ret;
}

obj_t **obj_var_create(obj_t *o, tstr_t *key)
{
    if (CLASS(o)->pre_var_create)
        CLASS(o)->pre_var_create(o, key);
    return var_create(&o->properties, key);
}

static obj_t *obj_new(unsigned char class)
{
    obj_t *ret = mem_cache_alloc(obj_cache[class - 1]);

    ret->class = class;
    ret->properties = NULL;
    ret->ref_count = 1;
    ret->flags = 0;
    return ret;
}

void _obj_set_property(obj_t *o, tstr_t property, obj_t *value)
{
    obj_t **dst;

    if (OBJ_IS_INT_VAL(o))
        return;

    if (CLASS(o)->set_own_property && 
        !CLASS(o)->set_own_property(o, property, value))
    {
        /* Handled */
        return;
    }

    dst = obj_var_create(o, &property);
    *dst = value;
}

int obj_get_int(obj_t *o)
{
    int ret;
    num_t *n;

    if (OBJ_IS_INT_VAL(o))
        return INT_VAL(o);

    n = to_num(obj_cast(o, NUM_CLASS));
    ret = NUM_IS_FP(n) ? (int)NUM_FP(n) : NUM_INT(n);
    obj_put(&n->obj);
    return ret;
}

double obj_get_fp(obj_t *o)
{
    double ret;
    num_t *n;

    n = to_num(obj_cast(o, NUM_CLASS));
    ret = num_fp_value(n);
    obj_put(&n->obj);
    return ret;
}

tstr_t obj_get_str(obj_t *o)
{
    tstr_t ret;
    string_t *s;

    s = to_string(obj_cast(o, STRING_CLASS));
    ret = tstr_dup(s->value);
    obj_put(&s->obj);
    return ret;
}

int obj_get_property_int(int *value, obj_t *o, const tstr_t *property)
{
    obj_t *p = obj_get_own_property(NULL, o, property);

    if (!p)
        return -1;

    *value = obj_get_int(p);
    obj_put(p);
    return 0;
}

void obj_inherit(obj_t *son, obj_t *parent)
{
    obj_t *func_proto;
    
    func_proto = obj_get_own_property(NULL, parent, &Sprototype);
    _obj_set_property(son, Sprototype, func_proto);
}

/*** "num" Class ***/

static void num_dump(printer_t *printer, obj_t *o)
{
    num_t *n = to_num(o);

    if (o == NAN_OBJ)
        tprintf(printer, "NaN", NUM_FP(n));
    else if (NUM_IS_FP(n))
        tprintf(printer, "%lf", NUM_FP(n));
    else
        tprintf(printer, "%d", NUM_INT(n));
}

double num_fp_value(num_t *n)
{
    return NUM_IS_FP(n) ? NUM_FP(n) : (double)NUM_INT(n);
}

static tstr_t num_to_tstr(num_t *n)
{
    if (NUM_IS_FP(n))
        return double_to_tstr(NUM_FP(n));
    else
        return int_to_tstr(NUM_INT(n));
}

static obj_t *num_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
        return string_new(num_to_tstr(to_num(o)));
    if (class == NUM_CLASS)
        return obj_get(o);

    return UNDEF;
}

static int fp_is_eq(double a, double b)
{
    double aa, bb, diff;

    aa = fabs(a);
    bb = fabs(b);
    diff = fabs(a - b);
    return diff <= MAX(aa, bb) * FLT_EPSILON ? 1 : 0;
}

static obj_t *num_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    num_t *a = to_num(oa), *b;
    obj_t *ret;
    int nan;
    
    /* If rvalue is a string, we perform string operation, unless lvalue
     * is ZERO, in which case, this is a unary operation which takes precedence 
     */
    if (oa != ZERO && is_string(ob))
        return obj_do_op(op, num_cast(oa, OBJ_CLASS(ob)), obj_get(ob));

    ob = CLASS(ob)->cast(ob, NUM_CLASS);
    b = to_num(ob);

    tp_info(("%s: op %x:%c oa %p ob %p\n", __FUNCTION__, op, op, oa, ob));

    nan = oa == NAN_OBJ || ob == NAN_OBJ;

    if (NUM_IS_FP(a) || NUM_IS_FP(b))
    {
        /* Floating point operations */
        double va = num_fp_value(a), vb = num_fp_value(b);

        switch (op)
        {
        case TOK_PLUS: ret = nan ? NAN_OBJ : num_new_fp(va + vb); break;
        case TOK_PLUS_PLUS: ret = nan ? NAN_OBJ : num_new_fp(va + 1); break;
        case TOK_MINUS: ret = nan ? NAN_OBJ : num_new_fp(va - vb); break;
        case TOK_MINUS_MINUS: ret = nan ? NAN_OBJ : num_new_fp(va - 1); break;
        case TOK_MULT: ret = nan ? NAN_OBJ : num_new_fp(va * vb); break;
        case TOK_DIV: ret = nan ? NAN_OBJ : num_new_fp(va / vb); break;
        case TOK_AND: ret = nan ? NAN_OBJ : num_new_int((int)va & (int)vb); break;
        case TOK_OR: ret = nan ? NAN_OBJ : num_new_int((int)va | (int)vb); break;
        case TOK_XOR: ret = nan ? NAN_OBJ : num_new_int((int)va ^ (int)vb); break;
        case TOK_GR: ret = !nan && (va > vb) ? TRUE : FALSE; break;
        case TOK_GE: ret = !nan && ((va > vb) || fp_is_eq(va, vb)) ? TRUE : FALSE; break;
        case TOK_LT: ret = !nan && (va < vb) ? TRUE : FALSE; break;
        case TOK_LE: ret = !nan && ((va < vb) || fp_is_eq(va, vb)) ? TRUE : FALSE; break;
        case TOK_IS_EQ: ret = !nan && fp_is_eq(va, vb) ? TRUE : FALSE; break;
        case TOK_NOT_EQ: ret = nan || !fp_is_eq(va, vb) ? TRUE : FALSE; break;
        default:
            ret = UNDEF;
            tp_crit(("OP %x:%c not defined for objs %p:%p\n", op, op, oa, ob));
        }
    }
    else
    {
        /* Integer operations */
        int va = NUM_INT(a), vb = NUM_INT(b);

        switch (op)
        {
        case TOK_PLUS: ret = nan ? NAN_OBJ : num_new_int(va + vb); break;
        case TOK_PLUS_PLUS: ret = nan ? NAN_OBJ : num_new_int(va + 1); break;
        case TOK_MINUS: ret = nan ? NAN_OBJ : num_new_int(va - vb); break;
        case TOK_MINUS_MINUS: ret = nan ? NAN_OBJ : num_new_int(va - 1); break;
        case TOK_TILDE: ret = nan ? NAN_OBJ : num_new_int(~(vb)); break;
        case TOK_MULT: ret = nan ? NAN_OBJ : num_new_int(va * vb); break;
        case TOK_DIV: ret = nan ? NAN_OBJ : num_new_fp((double)va / vb); break;
        case TOK_MOD: ret = nan ? NAN_OBJ : num_new_int(va % vb); break;
        case TOK_AND: ret = nan ? NAN_OBJ : num_new_int(va & vb); break;
        case TOK_OR: ret = nan ? NAN_OBJ : num_new_int(va | vb); break;
        case TOK_XOR: ret = nan ? NAN_OBJ : num_new_int(va ^ vb); break;
        case TOK_GR: ret = !nan && (va > vb) ? TRUE : FALSE; break;
        case TOK_GE: ret = !nan && (va >= vb) ? TRUE : FALSE; break;
        case TOK_LT: ret = !nan && (va < vb) ? TRUE : FALSE; break;
        case TOK_LE: ret = !nan && (va <= vb) ? TRUE : FALSE; break;
        case TOK_IS_EQ: ret = !nan && (va == vb) ? TRUE : FALSE; break;
        case TOK_NOT_EQ: ret = nan || (va != vb) ? TRUE : FALSE; break;
        case TOK_SHR: ret = nan ? NAN_OBJ : num_new_int(va >> vb); break;
        case TOK_SHRZ: ret = nan ? NAN_OBJ : num_new_int((unsigned int)va >> vb); break;
        case TOK_SHL: ret = nan ? NAN_OBJ : num_new_int(va << vb); break;
        default:
            ret = UNDEF;
            tp_crit(("OP %x:%c not defined for objs %p:%p\n", op, op, oa, ob));
        }
    }
    obj_put(ob);
    return ret;
}

static int num_is_true(obj_t *o)
{
    num_t *n = to_num(o);

    return NUM_IS_FP(n) ? !!NUM_FP(n) : !!NUM_INT(n);
}

#define NUM_BITS ((sizeof(uint_ptr_t) << 3))
#define INT_MSB (NUM_BITS - 2)
#define SIGN_BIT (NUM_BITS - 1)
obj_t *num_new_int(int v)
{
    num_t *ret;

    /* If shifting v left by one bit would destroy the sign bit,
     * a full num obj is required
     */
    if (!!(v & (1UL << SIGN_BIT)) != !!(v & (1UL << INT_MSB)))
    {
        ret = (num_t *)obj_new(NUM_CLASS);
        NUM_INT_SET(ret, v);
    }
    else
    {
        /* Number can be shifted and placed it in the pointer.
         * Mark the pointer as 'INT_VAL' using the pointer LSB
         * under the assumption that pointers are always aligned.
         */
        ret = (num_t *)(int_ptr_t)((v << 1) | 0x1);
    }

    return (obj_t *)ret;
}

obj_t *num_new_fp(double v)
{
    num_t *ret = (num_t *)obj_new(NUM_CLASS);

    NUM_SET_FP(ret);
    NUM_FP(ret) = v;
    return (obj_t *)ret;
}

obj_t *num_new(tnum_t n)
{
    return NUMERIC_IS_FP(n) ? num_new_fp(NUMERIC_FP(n)) : 
        num_new_int(NUMERIC_INT(n));
}

/*** "undefined" Class ***/

static obj_t *undefined_cast(obj_t *o, unsigned char class);

static void undefined_dump(printer_t *printer, obj_t *o)
{
    tprintf(printer, "undefined");
}

static obj_t *undefined_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    if (is_string(ob) || is_num(ob))
        return obj_do_op(op, undefined_cast(oa, OBJ_CLASS(ob)), obj_get(ob));

    switch (op)
    {
    case TOK_NOT_EQ: return (oa == ob || ob == NULL_OBJ) ? FALSE : TRUE;
    case TOK_IS_EQ: return (oa == ob || ob == NULL_OBJ) ? TRUE : FALSE;
    default:
        break;
    }
    return UNDEF;
}

static int undefined_is_true(obj_t *o)
{
    return 0;
}

static obj_t *undefined_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
        return string_new(S("undefined"));
    if (class == NUM_CLASS)
        return NAN_OBJ;

    return UNDEF;
}

/*** "null" Class ***/

static void null_dump(printer_t *printer, obj_t *o)
{
    tprintf(printer, "null");
}

static obj_t *null_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    switch (op)
    {
    case TOK_NOT_EQ: return (oa == ob || ob == UNDEF) ? FALSE : TRUE;
    case TOK_IS_EQ: return (oa == ob || ob == UNDEF) ? TRUE : FALSE;
    default:
        break;
    }
    return NULL_OBJ;
}

static int null_is_true(obj_t *o)
{
    return 0;
}

static obj_t *null_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
        return string_new(S("null"));
    if (class == NUM_CLASS)
        return num_new_int(0);

    return UNDEF;
}

/*** "bool" Class ***/

static int bool_is_true(obj_t *o)
{
    return ((bool_t *)o)->is_true;
}

static void bool_dump(printer_t *printer, obj_t *o)
{
    tprintf(printer, "%s", bool_is_true(o) ? "true" : "false");
}

static obj_t *bool_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
        return string_new(bool_is_true(o) ? S("true") : S("false"));
    if (class == NUM_CLASS)
        return num_new_int(bool_is_true(o));
    if (class == BOOL_CLASS)
        return obj_get(o);

    return UNDEF;
}

static obj_t *bool_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    if (is_num(ob) || is_string(ob))
        return obj_do_op(op, bool_cast(oa, OBJ_CLASS(ob)), obj_get(ob));

    switch (op)
    {
    case TOK_NOT_EQ:
        /* assuming there are not bool_t instances other than TRUE/FALSE */
        return oa != ob ? TRUE : FALSE;
    case TOK_IS_EQ:
        /* assuming there are not bool_t instances other than TRUE/FALSE */
        return oa == ob ? TRUE : FALSE;
    default:
        tp_crit(("OP %x:%c not defined for objs %p:%p\n", op, op, oa, ob));
    }
    return UNDEF;
}

/*** "function" class ***/

static obj_t *function_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
        return string_new(S("function"));
    if (class == FUNCTION_CLASS)
        return obj_get(o);

    return UNDEF;
}

static void function_dump(printer_t *printer, obj_t *o)
{
    function_t *function = to_function(o);
    tstr_list_t *l;

    tprintf(printer, "Function(");
    for (l = function->formal_params; l; l = l->next)
    {
        if (var_key_is_internal(&l->str))
            continue;

        tprintf(printer, "%S%s", &l->str, l->next ? ", " : "");
    }

    tprintf(printer, ")");
}

#ifdef CONFIG_OBJ_DOC
static int function_describe(printer_t *printer, obj_t *o)
{
    function_t *f = to_function(o);

    if (!f->doc.name)
        return -1;

    tprintf(printer, "Name: %s\n", f->doc.display_name ? : f->doc.name);
    tprintf(printer, "Description: %s\n", f->doc.description);
    if (f->doc.params[0].name)
    {
        const doc_function_param_t *p;
        
        tprintf(printer, "Parameters:\n");
        for (p = f->doc.params; p->name; p++)
            tprintf(printer, "\t%s: %s\n", p->name, p->description);
    }
    tprintf(printer, "Example:\n%s\n", f->doc.example);
    return 0;
}
#endif

static void function_free(obj_t *o)
{
    function_t *func = to_function(o);

    tstr_list_free(&func->formal_params);
    if (func->code_free_cb)
        func->code_free_cb(func->code);
    obj_put(func->scope);
}

static obj_t *function_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    obj_t *ret = NULL;

    switch (op)
    {
    case TOK_IS_EQ: 
        ret = oa == ob ? TRUE : FALSE;
        break;
    default:
        tp_crit(("OP %x:%c not defined for objs %p:%p\n", op, op, oa, ob));
    }
    return ret;
}

obj_t *function_new(tstr_list_t *params, void *code, code_free_cb_t code_free,
    obj_t *scope, call_t call)
{
    function_t *ret = (function_t *)obj_new(FUNCTION_CLASS);

    tp_assert(call);
    _obj_set_property(&ret->obj, Sprototype, object_new());
    ret->formal_params = params;
    ret->code = code;
    ret->code_free_cb = code_free;
    ret->scope = obj_get(scope);
    ret->call = call;
    return (obj_t *)ret;
}

int function_def_construct(obj_t **ret, obj_t *this_obj, int argc, 
    obj_t *argv[])
{
    int rc;
    function_t *func = to_function(argv[0]);

    this_obj = object_new();
    obj_inherit(this_obj, &func->obj);
    rc = func->call(ret, this_obj, argc, argv);
    if (rc == COMPLETION_THROW)
    {
        obj_put(this_obj);
        return rc;
    }

    if (is_function(*ret) || is_object(*ret) || is_array(*ret))
    {
        /* Functions and Objects are returned as-is by constructors */
        obj_put(this_obj);
    }
    else
    {
        obj_put(*ret);
        *ret = this_obj;
    }
    return rc;
}

int function_call_construct(obj_t **ret, int argc, obj_t *argv[])
{
    function_t *func = to_function(argv[0]);
    call_t c = func->obj.flags & OBJ_FUNCTION_CONSTRUCTOR ? 
        func->call : function_def_construct;

    return c(ret, UNDEF, argc, argv);
}

int function_call(obj_t **ret, obj_t *this_obj, int argc, obj_t *argv[])
{
    function_t *func = to_function(argv[0]);
    return func->call(ret, this_obj, argc, argv);
}

/*** "object" Class ***/

static void object_dump(printer_t *printer, obj_t *o)
{
    var_t *p;

    tprintf(printer, "{ ");
    for (p = o->properties; p; p = p->next)
    {
        if (var_key_is_internal(&p->key))
            continue;

        tprintf(printer, "%S : %o%s", &p->key, p->obj, p->next ?  ", " : "");
    }

    tprintf(printer, " }");
}

static obj_t *object_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
        return string_new(S("Object"));
    if (class == NUM_CLASS)
        return NAN_OBJ; /* XXX: not accurate, but will do */

    return UNDEF;
}

static obj_t *object_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    /* TODO: support object comparison to other types */
    switch (op)
    {
    case TOK_NOT_EQ:
        return oa != ob ? TRUE : FALSE;
    case TOK_IS_EQ:
        return oa == ob ? TRUE : FALSE;
    default:
        tp_crit(("OP %x:%c not defined for objs %p:%p\n", op, op, oa, ob));
    }
    return UNDEF;
}

void object_iter_init(object_iter_t *iter, obj_t *obj)
{
    iter->obj = obj;
    iter->key = NULL;
    iter->val = UNDEF;
    iter->priv = &obj->properties;
}

/* Returns 0 upon on the last element */
int object_iter_next(object_iter_t *iter)
{
    var_t *cur_prop;
    obj_t *proto;

    while ((cur_prop = *iter->priv))
    {
        iter->priv = &cur_prop->next;
        iter->key = &cur_prop->key;
        iter->val = cur_prop->obj;

        if (var_key_is_internal(iter->key))
            continue;

        return 1;
    }

    /* traverse our prototype */
    proto = obj_get_own_property(NULL, iter->obj, &Sprototype);
    if (!proto || proto == UNDEF)
        return 0;

    object_iter_init(iter, proto);
    obj_put(proto);
    return object_iter_next(iter);
}

void object_iter_uninit(object_iter_t *iter)
{
    iter->obj = NULL;
    iter->key = NULL;
    iter->val = UNDEF;
    iter->priv = NULL;
}

obj_t *object_new(void)
{
    obj_t *ret = obj_new(OBJECT_CLASS);
    return ret;
}

/*** "array" Class ***/

static obj_t **_array_length_get(int *length, obj_t *arr);

static void array_dump(printer_t *printer, obj_t *o)
{
    int first = 1, len, k, undef_streak = 0;

    if (obj_get_property_int(&len, o, &Slength))
    {
        tprintf(printer, "Unknown length array");
        return;
    }

    tprintf(printer, "[ ");
    for (k = 0; k < len; k++)
    {
        obj_t *item;

        if (!(item = array_lookup(o, k)))
        {
            undef_streak++;
            continue;
        }

        if (first)
            first = 0;
        else
            tprintf(printer, ", ");

        if (undef_streak)
        {
            tprintf(printer, "undefined x %d, ", undef_streak);
            undef_streak = 0;
        }

        obj_dump(printer, item);
        obj_put(item);
    }
    tprintf(printer, " ]");
}

static obj_t *array_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
    {
        obj_t *join, *ret;

        join = obj_get_property(NULL, o, &S("join"));

        if (!join || function_call(&ret, o, 1, &join))
            ret = string_new(S("Array"));

        obj_put(join);
        return ret;
    }
    return UNDEF;
}

static obj_t *array_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    switch (op)
    {
    case TOK_PLUS:
        return obj_do_op(op, obj_cast(oa, STRING_CLASS), obj_get(ob));
    default:
        break;
    }
    return object_do_op(op, oa, ob);
}

static obj_t **_array_length_get(int *length, obj_t *arr)
{
    obj_t **ret;

    tp_assert(is_array(arr));
    ret = var_get(arr->properties, &Slength);
    *length = NUM_INT(to_num(*ret));
    obj_put(*ret);
    return ret;
}

int array_length_get(obj_t *arr)
{
    int ret = 0;

    /* Intentionally fetch 'length' property not assuming this is an obj
     * of the 'array class'.
     */
    obj_get_property_int(&ret, arr, &Slength);
    return ret;
}

void array_length_set(obj_t *arr, int length)
{
    obj_t **len;
    int cur_len;

    len = _array_length_get(&cur_len, arr);

    /* Release old length */
    obj_put(*len);
    /* Set new length */
    *len = num_new_int(length);
}

obj_t *array_push(obj_t *arr, obj_t *item)
{
    obj_t **len, **dst;
    int idx;
    tstr_t idx_str;
    
    len = _array_length_get(&idx, arr);

    /* shortcut: we call var_create directly in order not to trigger
     * array_pre_var_create hook.
     */
    idx_str = int_to_tstr(idx);
    dst = var_create(&arr->properties, &idx_str);
    tstr_free(&idx_str);
    *dst = item;

    /* Release old length */
    obj_put(*len);
    /* Set new length */
    *len = num_new_int(idx + 1);
    return *len;
}

obj_t *array_pop(obj_t *arr)
{
    obj_t **len, *ret;
    int idx;
    var_t **iter, *tmp;
    tstr_t idx_id;
    
    len = _array_length_get(&idx, arr);

    if (idx == 0)
        return UNDEF;

    idx--;
    idx_id = int_to_tstr(idx);

    /* Lookup the last item */
    for (iter = &arr->properties; *iter && var_key_cmp(&(*iter)->key, &idx_id); 
        iter = &(*iter)->next);

    /* Release search str */
    tstr_free(&idx_id);

    if (!*iter)
        tp_crit(("Last element in array not found, this is weird...\n"));

    /* Keep reference to obj as we are returning it */
    ret = obj_get((*iter)->obj);

    /* Free the container */
    tmp = *iter;
    *iter = (*iter)->next;
    var_free(tmp);

    /* Release old length */
    obj_put(*len);
    /* Set new length */
    *len = num_new_int(idx);
    return ret;
}

obj_t *array_lookup(obj_t *arr, int index)
{
    tstr_t lookup_id;
    obj_t *ret;

    lookup_id = int_to_tstr(index);
    ret = obj_get_own_property(NULL, arr, &lookup_id);
    tstr_free(&lookup_id);
    return ret;
}

void array_iter_init(array_iter_t *iter, obj_t *arr, int reverse)
{
    int len = 0;

    len = array_length_get(arr);
    iter->len = len;
    iter->reverse = reverse;
    iter->k = reverse ? iter->len : -1;
    iter->arr = arr;
    iter->obj = NULL;
}

int array_iter_next(array_iter_t *iter)
{
    /* Release previous reference */
    if (iter->obj)
    {
        obj_put(iter->obj);
        iter->obj = NULL;
    }
    if (iter->reverse)
    {
        for (iter->k--; iter->k >= 0; iter->k--)
        {
            if ((iter->obj = array_lookup(iter->arr, iter->k)))
                break;
        }
    }
    else
    {
        for (iter->k++; iter->k < iter->len; iter->k++)
        {
            if ((iter->obj = array_lookup(iter->arr, iter->k)))
                break;
        }
    }

    return iter->reverse ? iter->k != -1 : iter->k != iter->len;
}

void array_iter_uninit(array_iter_t *iter)
{
    if (iter->obj)
    {
        obj_put(iter->obj);
        iter->obj = NULL;
    }
}

static void array_pre_var_create(obj_t *arr, const tstr_t *str)
{
    int idx = 0, cur_len;
    obj_t **len;
    tnum_t tnum_idx;
    
    if (tstr_to_tnum(&tnum_idx, str) || NUMERIC_IS_FP(tnum_idx))
    {
        /* Nothing better to do, it's ok to set a random property
         * on an array instance.
         */
        return;
    }
    
    idx = NUMERIC_INT(tnum_idx);
    len = _array_length_get(&cur_len, arr);

    if (cur_len > idx)
        return;

    /* Release old length */
    obj_put(*len);
    /* Set new length */
    *len = num_new_int(idx + 1);
}

obj_t *array_new(void)
{
    obj_t *ret = obj_new(ARRAY_CLASS), **len_prop;

    /* Add length property */
    len_prop = var_create(&ret->properties, &Slength);
    *len_prop = num_new_int(0);
    return ret;
}

/*** "env" Class ***/

static void env_dump(printer_t *printer, obj_t *o)
{
    var_t *p;

    tprintf(printer, "{ ");
    for (p = o->properties; p; p = p->next)
    {
        tprintf(printer, "%S : %o [refs %d]%s", &p->key, p->obj, 
            OBJ_IS_INT_VAL(p->obj) ? 1 : p->obj->ref_count, p->next ?  ", " : 
            "");
    }

    tprintf(printer, " }");
}

obj_t *env_new(obj_t *env)
{
    env_t *n = (env_t *)obj_new(ENV_CLASS);

    if (env)
        obj_set_property(&n->obj, Sprototype, env);
    return (obj_t *)n;
}

/*** "string" Class ***/

static void string_dump(printer_t *printer, obj_t *o)
{
    string_t *s = to_string(o);

    tprintf(printer, "\"%S\"", &s->value);
}

static void string_free(obj_t *o)
{
    string_t *s = to_string(o);

    tstr_free(&s->value);
}

static obj_t *string_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    obj_t *ret = NULL;
    string_t *a, *b;

    a = to_string(oa);
    ob = CLASS(ob)->cast(ob, STRING_CLASS);
    b = to_string(ob);

    switch (op)
    {
#define COMPARE(op, cond) \
    case op: {\
        int diff = tstr_cmp(&a->value, &b->value); \
        ret = cond ? TRUE : FALSE; \
    } break
    COMPARE(TOK_NOT_EQ, diff != 0);
    COMPARE(TOK_IS_EQ, diff == 0);
    COMPARE(TOK_GR, diff > 0);
    COMPARE(TOK_LT, diff < 0);
    COMPARE(TOK_GE, diff >= 0);
    COMPARE(TOK_LE, diff <= 0);
    case TOK_PLUS:
        {
            tstr_t s;

            tstr_cat(&s, &a->value, &b->value);
            ret = string_new(s);
        }
        break;
    default:
        tp_crit(("OP %x:%c not defined for objs %p:%p\n", op, op, oa, ob));
    }
    obj_put(ob);
    return ret;
}

static obj_t *string_to_num(string_t *s)
{
    tnum_t value;

    if (tstr_to_tnum(&value, &s->value))
        return NAN_OBJ;

    return num_new(value);
}

static obj_t *string_to_bool(string_t *s)
{
    return s->value.len ? TRUE : FALSE;
}

static obj_t *string_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
        return obj_get(o);
    if (class == NUM_CLASS)
        return string_to_num(to_string(o));
    if (class == BOOL_CLASS)
        return string_to_bool(to_string(o));

    return UNDEF;
}

static int string_is_true(obj_t *o)
{
    return string_cast(o, BOOL_CLASS) == TRUE;
}

static obj_t *string_get_own_property(obj_t ***lval, obj_t *o,
    const tstr_t *str)
{
    tnum_t tidx;
    int idx;
    string_t *s = to_string(o);
    tstr_t retval;

    if (tstr_to_tnum(&tidx, str))
        return NULL;

    /* XXX: tstr_to_int? */
    idx = NUMERIC_INT(tidx);
    if (s->value.len <= idx)
        return NULL;

    retval = tstr_slice(s->value, idx, 1);
    if (lval)
        *lval = NULL;
    return string_new(retval);
}

obj_t *string_new(tstr_t s)
{
    string_t *ret = (string_t *)obj_new(STRING_CLASS);
    obj_t **len_prop;

    ret->value = s;

    /* Add length property */
    len_prop = var_create(&ret->obj.properties, &Slength);
    *len_prop = num_new_int(s.len);

    return (obj_t *)ret;
}

/*** "typed arrays" classes ***/

#define SbyteLength S("byteLength")

static obj_t *array_buffer_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
        return string_new(S("[object ArrayBuffer]"));

    return UNDEF;
}
static obj_t *array_buffer_get_own_property(obj_t ***lval, obj_t *o, 
    const tstr_t *str)
{
    array_buffer_t *b = to_array_buffer(o);

    if (!tstr_cmp(str, &SbyteLength))
    {
        if (lval)
            *lval = NULL;
        return num_new_int(b->value.len);
    }

    return NULL;
}

static void array_buffer_dump(printer_t *printer, obj_t *o)
{
    array_buffer_t *b = to_array_buffer(o);

    tprintf(printer, "ArrayBuffer(%d)", b->value.len);
}

static void array_buffer_free(obj_t *o)
{
    array_buffer_t *b = to_array_buffer(o);

    tstr_free(&b->value);
}

obj_t *array_buffer_new(int length)
{
    array_buffer_t *ret = (array_buffer_t *)obj_new(ARRAY_BUFFER_CLASS);
    
    tstr_zalloc(&ret->value, length);
    return (obj_t *)ret;
}

static obj_t *array_buffer_view_cast(obj_t *o, unsigned char class)
{
    array_buffer_view_t *v = to_array_buffer_view(o);

    if (class == STRING_CLASS)
    {
        int shift = v->flags & ABV_SHIFT_MASK;

        switch (shift)
        {
        case 0:
            if (v->flags & ABV_FLAG_UNSIGNED)
                return string_new(S("[object Uint8Array]"));
            else
                return string_new(S("[object int8Array]"));
        case 1:
            if (v->flags & ABV_FLAG_UNSIGNED)
                return string_new(S("[object Uint16Array]"));
            else
                return string_new(S("[object int16Array]"));
        case 2:
            if (v->flags & ABV_FLAG_UNSIGNED)
                return string_new(S("[object Uint32Array]"));
            else
                return string_new(S("[object int32rray]"));
        default:
            return string_new(S("ArrayBufferView"));
        }
    }

    return UNDEF;
}

int array_buffer_view_item_val_get(array_buffer_view_t *v, int idx)
{
    int shift;
    char buf[sizeof(u64)]; /* Maximal type size */
    char *bval = buf;

    idx += v->offset;
    shift = v->flags & ABV_SHIFT_MASK;
    tstr_serialize(bval, &v->array_buffer->value, idx << shift, 1 << shift);
    switch (shift)
    {
    case 0:
        if (v->flags & ABV_FLAG_UNSIGNED)
            return (int)*(u8 *)bval;
        else
            return (int)*(s8 *)bval;
    case 1:
        if (v->flags & ABV_FLAG_UNSIGNED)
            return (int)*((u16 *)bval);
        else
            return (int)*((s16 *)bval);
    case 2:
        if (v->flags & ABV_FLAG_UNSIGNED)
            return (int)*(u32 *)bval;
        else
            return (int)*(s32 *)bval;
    }
    return 0;
}

int array_buffer_view_item_val_set(array_buffer_view_t *v, int idx, int val)
{
    int shift;
    tstr_t bval;

    shift = v->flags & ABV_SHIFT_MASK;
    idx += v->offset;
    bval = tstr_piece(v->array_buffer->value, idx << shift, 1 << shift);
    switch (shift)
    {
    case 0:
        if (v->flags & ABV_FLAG_UNSIGNED)
            *(u8 *)TPTR(&bval) = (u8)val;
        else
            *(s8 *)TPTR(&bval) = (s8)val;
        break;
    case 1:
        if (v->flags & ABV_FLAG_UNSIGNED)
            *(u16 *)TPTR(&bval) = (u16)val;
        else
            *(s16 *)TPTR(&bval) = (s16)val;
        break;
    case 2:
        if (v->flags & ABV_FLAG_UNSIGNED)
            *(u32 *)TPTR(&bval) = (u32)val;
        else
            *(s32 *)TPTR(&bval) = (s32)val;
        break;
    default:
        return -1;
    }
    return 0;
}

static obj_t *array_buffer_view_get_own_property(obj_t ***lval, obj_t *o, 
    const tstr_t *str)
{
    array_buffer_view_t *v = to_array_buffer_view(o);
    tnum_t tidx;
    int retval, idx;

    if (!tstr_cmp(str, &Slength))
    {
        retval = v->length;
        goto Ok;
    }
    if (!tstr_cmp(str, &S("BYTES_PER_ELEMENT")))
    {
        retval = 1 << (v->flags & ABV_SHIFT_MASK);
        goto Ok;
    }

    if (!tstr_cmp(str, &S("buffer")))
    {
        *lval = NULL;
        return obj_get((obj_t *)v->array_buffer);
    }

    if (tstr_to_tnum(&tidx, str))
        return NULL;

    idx = NUMERIC_INT(tidx);
    if (v->length < idx)
        return NULL;

    retval = array_buffer_view_item_val_get(v, idx);

Ok:
    if (lval)
        *lval = NULL;
    return num_new_int(retval);
}

static int array_buffer_view_set_own_property(obj_t *o, tstr_t str,
    obj_t *value)
{
    array_buffer_view_t *v = to_array_buffer_view(o);
    tnum_t tidx;
    int val, idx;

    if (tstr_to_tnum(&tidx, &str) || NUMERIC_IS_FP(tidx))
    {
        /* Allow creating arbitrary properties on typed arrays */
        return -1;
    }

    idx = NUMERIC_INT(tidx);
    if (v->length < idx)
    {
        /* Out of range indices are ignored */
        return 0;
    }

    val = obj_get_int(value);
    /* value is no longer needed. We do not really store a reference to it */
    obj_put(value);

    return array_buffer_view_item_val_set(v, idx, val);
}

static void array_buffer_view_free(obj_t *o)
{
    array_buffer_view_t *v = to_array_buffer_view(o);

    obj_put((obj_t *)v->array_buffer);
}

obj_t *array_buffer_view_new(obj_t *array_buffer, u32 flags, u32 offset,
    int length)
{
    array_buffer_view_t *ret = (array_buffer_view_t *)obj_new(
        ARRAY_BUFFER_VIEW_CLASS);

    ret->array_buffer = (array_buffer_t *)obj_get(array_buffer);
    ret->flags = flags;
    ret->offset = offset;
    ret->length = length;
    return (obj_t *)ret;
}

/*** "arguments" Class ***/
obj_t *arguments_new(function_args_t *args)
{
    arguments_t *ret = (arguments_t *)obj_new(ARGUMENTS_CLASS);
    function_args_t *dst = &ret->args;
    int i;

    /* argv[0] is the called function. Don't copy it */
    dst->argc = args->argc - 1;
    dst->argv = tmalloc(dst->argc * sizeof(obj_t *), "Cloned Args");
    for (i = 0; i < dst->argc; i++)
        dst->argv[i] = obj_get(args->argv[i + 1]);

    return (obj_t *)ret;
}

static void arguments_free(obj_t *o)
{
    arguments_t *arguments = to_arguments(o);
    int i;

    /* XXX: perhaps always take a reference when adding objs to function args,
     * this way we can alway put them on release - move this code to
     * function_args_uninit()...
     */
    for (i = 0; i < arguments->args.argc; i++)
        obj_put(arguments->args.argv[i]);
    
    function_args_uninit(&arguments->args);
}

static obj_t *arguments_get_own_property(obj_t ***lval, obj_t *o, 
    const tstr_t *str)
{
    arguments_t *arguments = to_arguments(o);
    tnum_t tidx;
    int idx;

    if (lval)
        *lval = NULL;

    if (!tstr_cmp(str, &Slength))
        return num_new_int(arguments->args.argc);

    if (tstr_to_tnum(&tidx, str))
        return NULL;

    idx = NUMERIC_INT(tidx);
    if (idx < 0 || idx > arguments->args.argc - 1)
        return NULL;

    return obj_get(arguments->args.argv[idx]);
}

/*** "pointer" Class ***/
obj_t *pointer_new(void *ptr, void (*free)(void *ptr))
{
    pointer_t *ret = (pointer_t *)obj_new(POINTER_CLASS);
    
    ret->ptr = ptr;
    ret->free = free;
    return (obj_t *)ret;
}

static void pointer_free(obj_t *o)
{
    pointer_t *p = to_pointer(o);

    if (p->free)
        p->free(p->ptr);
}

static void pointer_dump(printer_t *printer, obj_t *o)
{
    pointer_t *p = to_pointer(o);

    tprintf(printer, "[0x%p]", p->ptr);
}

/*** Initialization Sequence Functions ***/
void obj_class_set_prototype(unsigned char class, obj_t *proto)
{
    class_prototypes[class] = proto;
}

void js_obj_uninit(void)
{
    int i;

    for (i = 0; i < CLASS_LAST; i++)
    {
        if (obj_cache[i])
            mem_cache_destroy(obj_cache[i]);
    }
    mem_cache_destroy(var_cache);
}

void js_obj_init(void)
{
    var_cache = mem_cache_create_type(var_t);
#define OBJ_CACHE_INIT(type, class) \
    obj_cache[class - 1] = mem_cache_create_type(type)
    OBJ_CACHE_INIT(num_t, NUM_CLASS);
    OBJ_CACHE_INIT(function_t, FUNCTION_CLASS);
    OBJ_CACHE_INIT(string_t, STRING_CLASS);
    OBJ_CACHE_INIT(obj_t, OBJECT_CLASS);
    OBJ_CACHE_INIT(obj_t, ARRAY_CLASS);
    OBJ_CACHE_INIT(env_t, ENV_CLASS);
    OBJ_CACHE_INIT(array_buffer_t, ARRAY_BUFFER_CLASS);
    OBJ_CACHE_INIT(array_buffer_view_t, ARRAY_BUFFER_VIEW_CLASS);
    OBJ_CACHE_INIT(arguments_t, ARGUMENTS_CLASS);
    OBJ_CACHE_INIT(pointer_t, POINTER_CLASS);
}

const obj_class_t classes[] = {
    [ NUM_CLASS ] = {
        .dump = num_dump,
        .do_op = num_do_op,
        .is_true = num_is_true,
        .cast = num_cast,
    },
    [ FUNCTION_CLASS ] = {
        .dump = function_dump,
#ifdef CONFIG_OBJ_DOC
        .describe = function_describe,
#endif
        .free = function_free,
        .do_op = function_do_op,
        .cast = function_cast,
    },
    [ UNDEFINED_CLASS ] = {
        .dump = undefined_dump,
        .do_op = undefined_do_op,
        .is_true = undefined_is_true,
        .cast = undefined_cast,
    },
    [ NULL_CLASS ] = {
        .dump = null_dump,
        .do_op = null_do_op,
        .is_true = null_is_true,
        .cast = null_cast,
    },
    [ BOOL_CLASS ] = {
        .dump = bool_dump,
        .do_op = bool_do_op,
        .is_true = bool_is_true,
        .cast = bool_cast,
    },
    [ STRING_CLASS ] = {
        .dump = string_dump,
        .free = string_free,
        .do_op = string_do_op,
        .cast = string_cast,
        .is_true = string_is_true,
        .get_own_property = string_get_own_property,
    },
    [ OBJECT_CLASS ] = {
        .dump = object_dump,
        .cast = object_cast,
        .do_op = object_do_op,
    },
    [ ARRAY_CLASS] = {
        .dump = array_dump,
        .cast = array_cast,
        .do_op = array_do_op,
        .pre_var_create = array_pre_var_create,
    },
    [ ENV_CLASS ] = {
        .dump = env_dump,
    },
    [ ARRAY_BUFFER_CLASS ] = {
        .dump = array_buffer_dump,
        .cast = array_buffer_cast,
        .free = array_buffer_free,
        .get_own_property = array_buffer_get_own_property,
        .do_op = object_do_op,
    },
    [ ARRAY_BUFFER_VIEW_CLASS ] = {
        .dump = array_dump,
        .cast = array_buffer_view_cast,
        .free = array_buffer_view_free,
        .get_own_property = array_buffer_view_get_own_property,
        .set_own_property = array_buffer_view_set_own_property,
        .do_op = object_do_op,
    },
    [ ARGUMENTS_CLASS ] = {
        .dump = array_dump,
        .free = arguments_free,
        .get_own_property = arguments_get_own_property,
        .do_op = object_do_op,
    },
    [ POINTER_CLASS ] = {
        .dump = pointer_dump,
        .free = pointer_free,
        .do_op = object_do_op,
    },
};
