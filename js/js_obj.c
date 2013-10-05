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
#include <stdlib.h>
#include <string.h>
#include "util/tmalloc.h"
#include "util/tprintf.h"
#include "util/debug.h"
#include "js/js_obj.h"
#include "js/js_types.h"

#define Slength INTERNAL_S("length")

struct obj_class_t {
    void (*dump)(printer_t *printer, obj_t *o);
    void (*free)(obj_t *o);
    obj_t *(*do_op)(token_type_t op, obj_t *oa, obj_t *ob);
    int (*is_true)(obj_t *o);
    obj_t *(*cast)(obj_t *o, unsigned char class);
    void (*pre_var_create)(obj_t *o, const tstr_t *str);
    obj_t *(*get_own_property)(obj_t ***lval, obj_t *o, tstr_t str);
};

/* classes is defined at the bottom of this file.
 * It 'extern' instead of 'static' to avoid a mass of forward declarations
 */
extern const obj_class_t classes[];

static obj_t *class_prototypes[CLASS_LAST+1];

#define CLASS(obj) (&classes[(obj)->class])
#define CLASS_PROTOTYPE(obj) (class_prototypes[(obj)->class])

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

static inline int var_key_cmp(tstr_t *keya, tstr_t *keyb)
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
    tfree(v);
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

static obj_t **var_get(var_t *vars, tstr_t key)
{
    var_t *iter;

    for (iter = vars; iter && var_key_cmp(&iter->key, &key); iter = iter->next);
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
	*iter = tmalloc_type(var_t);
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
	tfree(o);
}

obj_t *obj_get_property(obj_t ***lval, obj_t *o, tstr_t property)
{
    obj_t **ref = NULL, *val = NULL, *proto;

    tp_debug(("Lookup %S in obj %p\n", &property, o));
    if ((val = obj_get_own_property(&ref, o, property)))
	goto Exit;

    proto = obj_get_own_property(NULL, o, Sprototype);
    if (proto && proto != UNDEF)
    {
	val = obj_get_property(&ref, proto, property);
	obj_put(proto);
    }

    /* If this is an env obj, there is nothing more we can do. */
    if (is_env(o))
	goto Exit;

    /* If we couldn't find a better match, let's lookup the class_prototype,
     * Note that the class prototype of the "Object" class prototypes leads
     * to itself...
     */
    if (!val && CLASS_PROTOTYPE(o) && CLASS_PROTOTYPE(o) != o)
    {
        val = obj_get_property(&ref, CLASS_PROTOTYPE(o), property);
        /* User is not allowed to change the class prototype */
        ref = NULL;
    }

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

obj_t *obj_get_own_property(obj_t ***lval, obj_t *o, tstr_t key)
{
    obj_t **ref;

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

obj_t *obj_has_property(obj_t *o, tstr_t property)
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
    ret = obj_has_property(ob, property);
    tstr_free(&property);
    return ret;
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
    case TOK_NOT_EQ_STRICT:
    case TOK_IS_EQ_STRICT:
	if (CLASS(oa) != CLASS(ob))
	{
	    ret = op == TOK_NOT_EQ_STRICT ? TRUE : FALSE;
	    break;
	}
	/* Fallthrough */
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

obj_t *obj_new(unsigned char class, int size, char *type)
{
    obj_t *ret = tmalloc(size, type);

    ret->class = class;
    ret->properties = NULL;
    ret->ref_count = 1;
    ret->flags = 0;
    return ret;
}

void _obj_set_property(obj_t *o, tstr_t property, obj_t *value)
{
    obj_t **dst;

    dst = obj_var_create(o, &property);
    *dst = value;
}

void obj_set_property_str(obj_t *o, tstr_t property, tstr_t value)
{
    _obj_set_property(o, property, string_new(value));
}

void obj_set_property_int(obj_t *o, tstr_t property, int value)
{
    _obj_set_property(o, property, num_new_int(value));
}

int obj_get_int(obj_t *o)
{
    int ret;
    num_t *n;

    n = to_num(obj_cast(o, NUM_CLASS));
    ret = NUM_INT(n);
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

int obj_get_property_int(int *value, obj_t *o, tstr_t property)
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
    
    func_proto = obj_get_own_property(NULL, parent, Sprototype);
    _obj_set_property(son, Sprototype, func_proto);
}

int throw_exception(obj_t **po, tstr_t *desc)
{
    obj_put(*po);
    *po = string_new(*desc);
    return COMPLETION_THROW;
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

static obj_t *num_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    num_t *a = to_num(oa), *b;
    obj_t *ret;
    int nan;
    
    /* If rvalue is a string, we perform string operation, unless lvalue
     * is ZERO, in which case, this is a unary operation which takes precedence 
     */
    if (oa != ZERO && is_string(ob))
    {
	obj_t *ret, *n = num_cast(oa, STRING_CLASS);
	ret = string_do_op(op, n, ob);
	obj_put(n);
	return ret;
    }

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
	case TOK_GR: ret = !nan && (va > vb) ? TRUE : FALSE; break;
	case TOK_GE: ret = !nan && (va >= vb) ? TRUE : FALSE; break;
	case TOK_LT: ret = !nan && (va < vb) ? TRUE : FALSE; break;
	case TOK_LE: ret = !nan && (va <= vb) ? TRUE : FALSE; break;
	case TOK_IS_EQ: ret = !nan && (va == vb) ? TRUE : FALSE; break;
	case TOK_NOT_EQ: ret = nan || (va != vb) ? TRUE : FALSE; break;
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

    /* Use integer all the time for this... 
     * XXX: should we panic on FP? */
    return NUM_INT(n) ? 1 : 0;
}

obj_t *num_new_int(int v)
{
    num_t *ret = (num_t *)obj_new_type(NUM_CLASS, num_t);

    NUM_INT(ret) = v;
    return (obj_t *)ret;
}

obj_t *num_new_fp(double v)
{
    num_t *ret = (num_t *)obj_new_type(NUM_CLASS, num_t);

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

static void undefined_dump(printer_t *printer, obj_t *o)
{
    tprintf(printer, "undefined");
}

static obj_t *undefined_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
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
    if (is_num(ob))
    {
	obj_t *ret, *n = bool_cast(oa, NUM_CLASS);
	ret = num_do_op(op, n, ob);
	obj_put(n);
	return ret;
    }

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

static void function_free(obj_t *o)
{
    function_t *func = to_function(o);

    tstr_list_free(&func->formal_params);
    js_scan_free(func->code);
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

obj_t *function_new(tstr_list_t *params, scan_t *code, obj_t *scope, 
    call_t call)
{
    function_t *ret = (function_t *)obj_new_type(FUNCTION_CLASS, function_t);

    tp_assert(call);
    _obj_set_property(&ret->obj, Sprototype, object_new());
    ret->formal_params = params;
    ret->code = code;
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
	return rc;

    obj_put(*ret);
    *ret = this_obj;
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
	tprintf(printer, "%S : %o%s", &p->key, p->obj, p->next ?  ", " : "");

    tprintf(printer, " }");
}

static obj_t *object_cast(obj_t *o, unsigned char class)
{
    if (class == STRING_CLASS)
	return string_new(S("Object"));
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
    proto = obj_get_own_property(NULL, iter->obj, Sprototype);
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
    obj_t *ret = obj_new_type(OBJECT_CLASS, obj_t);
    return ret;
}

/*** "array" Class ***/

static obj_t **array_length_get(int *length, obj_t *arr);
static obj_t *array_lookup(obj_t *arr, int index);

static void array_dump(printer_t *printer, obj_t *o)
{
    int first = 1, len, k, undef_streak = 0;

    array_length_get(&len, o);
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
	return string_new(S("Array"));
    return UNDEF;
}

static obj_t *array_do_op(token_type_t op, obj_t *oa, obj_t *ob)
{
    return object_do_op(op, oa, ob);
}

static obj_t **array_length_get(int *length, obj_t *arr)
{
    obj_t **ret;

    ret = var_get(arr->properties, Slength);
    *length = NUM_INT(to_num(*ret));
    obj_put(*ret);
    return ret;
}

void array_length_set(obj_t *arr, int length)
{
    obj_t **len;
    int cur_len;

    len = array_length_get(&cur_len, arr);

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
    
    len = array_length_get(&idx, arr);

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
    
    len = array_length_get(&idx, arr);

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

static obj_t *array_lookup(obj_t *arr, int index)
{
    tstr_t lookup_id;
    obj_t *ret;

    lookup_id = int_to_tstr(index);
    ret = obj_get_own_property(NULL, arr, lookup_id);
    tstr_free(&lookup_id);
    return ret;
}

void array_iter_init(array_iter_t *iter, obj_t *arr, int reverse)
{
    int len;

    array_length_get(&len, arr);
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
    len = array_length_get(&cur_len, arr);

    if (cur_len > idx)
	return;

    /* Release old length */
    obj_put(*len);
    /* Set new length */
    *len = num_new_int(idx + 1);
}

obj_t *array_new(void)
{
    obj_t *ret = obj_new_type(ARRAY_CLASS, obj_t), **len_prop;

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
	    p->obj->ref_count, p->next ?  ", " : "");
    }

    tprintf(printer, " }");
}

obj_t *env_new(obj_t *env)
{
    env_t *n = (env_t *)obj_new_type(ENV_CLASS, env_t);

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
    case TOK_NOT_EQ: 
	ret = tstr_cmp(&a->value, &b->value) ? TRUE : FALSE;
	break;
    case TOK_IS_EQ: 
	ret = !tstr_cmp(&a->value, &b->value) ? TRUE : FALSE;
	break;
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

static obj_t *string_get_own_property(obj_t ***lval, obj_t *o, tstr_t str)
{
    tnum_t tidx;
    int idx;
    string_t *s = to_string(o);
    tstr_t retval;

    if (tstr_to_tnum(&tidx, &str))
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
    string_t *ret = (string_t *)obj_new_type(STRING_CLASS, string_t);
    obj_t **len_prop;

    ret->value = s;

    /* Add length property */
    len_prop = var_create(&ret->obj.properties, &Slength);
    *len_prop = num_new_int(s.len);

    return (obj_t *)ret;
}

/*** Initialization Sequence Functions ***/
void obj_class_set_prototype(unsigned char class, obj_t *proto)
{
    class_prototypes[class] = proto;
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
};
