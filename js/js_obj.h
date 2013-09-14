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
#ifndef __JS_OBJ_H__
#define __JS_OBJ_H__

#include "js/js_scan.h"
#include "util/tprintf.h"
#include "util/debug.h"

typedef struct obj_t obj_t;
typedef struct var_t var_t;
typedef struct obj_class_t obj_class_t;
typedef struct function_t function_t;

#define Sprototype S("prototype")

struct obj_t {
    /* Trick : We use flags in obj_t for subclasses purposes - ugly, but
     *   saves space.
     */
#define OBJ_STATIC 0x0001
#define OBJ_NUM_FP 0x0002
    /* Use 'call' as a construct routine instead of the default
     * 'construct' function
     */
#define OBJ_FUNCTION_CONSTRUCTOR 0x0004
    unsigned char flags;
    unsigned char class;
    char ref_count;
    unsigned char reserved;
    var_t *properties;
};

typedef int (*call_t)(obj_t **ret, obj_t *this, int argc, obj_t *argv[]);

struct function_t {
    obj_t obj;
    call_t call;
    scan_t *code;
    obj_t *scope;
    tstr_list_t *formal_params;
};

typedef struct {
    obj_t obj;
    union {
	int i;
	double fp;
    } value;
} num_t;

#define NUM_IS_FP(x) ((x)->obj.flags & OBJ_NUM_FP)
#define NUM_SET_FP(x) ((x)->obj.flags |= OBJ_NUM_FP)
#define NUM_INT(x) ((x)->value.i)
#define NUM_FP(x) ((x)->value.fp)

typedef struct {
    obj_t obj;
    tstr_t value;
} string_t;

typedef struct {
    obj_t obj;
    int is_true;
} bool_t;

struct var_t {
    var_t *next;
    tstr_t str; /* XXX: rename (key?) */
    obj_t *obj;
};

typedef struct {
    obj_t obj;
} env_t;

/* Class types.
 * Note: ENV_CLASS is a special class - it is not exposed as a JS type,
 * but shares a lot of common properties with other classes.
 */
#define NUM_CLASS 1
#define FUNCTION_CLASS 2
#define UNDEFINED_CLASS 3
#define NULL_CLASS 4
#define BOOL_CLASS 5
#define STRING_CLASS 6
#define OBJECT_CLASS 7
#define ARRAY_CLASS 8
#define ENV_CLASS 9
#define CLASS_LAST ENV_CLASS

/* Global objects */
extern obj_t undefind_obj;
#define UNDEF (&undefind_obj)
extern obj_t null_obj;
#define NULL_OBJ (&null_obj)
extern num_t zero_obj;
extern num_t nan_obj;
#define ZERO ((obj_t *)&zero_obj)
#define NAN_OBJ ((obj_t *)&nan_obj)
extern bool_t true_obj;
extern bool_t false_obj;
#define TRUE ((obj_t *)&true_obj)
#define FALSE ((obj_t *)&false_obj)

#define _STATIC_OBJ(c, f) { .ref_count = 1, .flags = OBJ_STATIC | f, \
    .properties = NULL, .class = c }

#define STATIC_OBJ(c) _STATIC_OBJ(c, 0)

#define STATIC_FUNCTION(f) { .obj = STATIC_OBJ(FUNCTION_CLASS), \
    .call = f, .code = NULL, .scope = NULL, .formal_params = NULL }
#define STATIC_CONSTRUCTOR(f) { \
    .obj = _STATIC_OBJ(FUNCTION_CLASS, OBJ_FUNCTION_CONSTRUCTOR), \
    .call = f, .code = NULL, .scope = NULL, .formal_params = NULL }

#define STATIC_STRING(s) { .obj = STATIC_OBJ(STRING_CLASS), .value = S(s) }
#define STATIC_NUM(v) { .obj = STATIC_OBJ(NUM_CLASS), .value.i = v }
#define STATIC_NUM_FP(v) { .obj = _STATIC_OBJ(NUM_CLASS, OBJ_NUM_FP), \
    .value.fp = v }

/* Generic obj methods */
obj_t *obj_cast(obj_t *o, unsigned char class);
obj_t **obj_var_create(obj_t *o, tstr_t str);
obj_t *obj_get_own_property(obj_t ***lval, obj_t *o, tstr_t str);
obj_t *obj_get_property(obj_t ***lval, obj_t *o, tstr_t property);
obj_t *obj_do_op(token_type_t op, obj_t *oa, obj_t *ob);
obj_t *obj_new(unsigned char class, int size, char *type);
#define obj_new_type(c, type) obj_new(c, sizeof(type), #type)

static inline obj_t *obj_get(obj_t *o)
{
    if (!o)
	return NULL;

    /* We save up space by using a single byte for ref_count,
     * XXX: static objects may have negative ref_count (?)
     */
    tp_assert(o->ref_count < 255 || o->flags & OBJ_STATIC);
    o->ref_count++;
    return o;
}

void _obj_put(obj_t *o);

static inline void obj_put(obj_t *o)
{
    if (!o || o == UNDEF)
	return;

    if (--o->ref_count > 0)
	return;

    _obj_put(o);
}

void obj_dump(printer_t *printer, obj_t *obj);
int obj_true(obj_t *o);

/* "num" objects methods */
obj_t *num_new_fp(double v);
obj_t *num_new_int(int v);
obj_t *num_new(tnum_t n);

static inline int is_num(obj_t *o)
{
    return o && o->class == NUM_CLASS;
}

static inline num_t *to_num(obj_t *o)
{
    tp_assert(is_num(o));
    return (num_t *)o;
}

/* "function" objects methods */
obj_t *function_new(tstr_list_t *params, scan_t *code, obj_t *scope, 
    call_t call);
int function_call(obj_t **ret, obj_t *this_obj, int argc, obj_t *argv[]);
int function_call_construct(obj_t **ret, int argc, obj_t *argv[]);

static inline int is_function(obj_t *o)
{
    return o && o->class == FUNCTION_CLASS;
}

static inline function_t *to_function(obj_t *o)
{
    tp_assert(is_function(o));
    return (function_t *)o;
}

/* "string" objects methods */
obj_t *string_new(tstr_t s);

static inline int is_string(obj_t *o)
{
    return o && o->class == STRING_CLASS;
}

static inline string_t *to_string(obj_t *o)
{
    tp_assert(is_string(o));
    return (string_t *)o;
}

/* "object" objects methods */
obj_t *object_new(void);

typedef struct {
    obj_t *obj; /* Iterated object */
    tstr_t *key; /* Key of the current interated item */
    obj_t *val; /* The current iterated item */
    var_t **priv;
} object_iter_t;

void object_iter_init(object_iter_t *iter, obj_t *obj);
/* Returns 0 upon on the last element */
int object_iter_next(object_iter_t *iter);
void object_iter_uninit(object_iter_t *iter);

/* "env" objects methods */
/* env_new takes a reference to the parent env */
obj_t *env_new(obj_t *env);

static inline int is_env(obj_t *o)
{
    return o && o->class == ENV_CLASS;
}

/* "array" objects methods */
obj_t *array_new(void);
obj_t *array_push(obj_t *arr, obj_t *item);
obj_t *array_pop(obj_t *arr);
void array_length_set(obj_t *arr, int length);

static inline int is_array(obj_t *o)
{
    return o && o->class == ARRAY_CLASS;
}

typedef struct {
    obj_t *arr; /* Iterated array */
    obj_t *obj; /* The current iterated item */
    int k; /* Index of the current iterated item */
    int len; /* Array length */
    int reverse; /* Iterate from length-1 to 0 */
} array_iter_t;

void array_iter_init(array_iter_t *iter, obj_t *arr, int reverse);
/* Returns 0 upon on the last element */
int array_iter_next(array_iter_t *iter);
void array_iter_uninit(array_iter_t *iter);

/* Initialization sequence functions */
void obj_class_set_prototype(unsigned char class, obj_t *proto);

/* Utility functions for obj property manipulation */
void _obj_set_property(obj_t *o, tstr_t property, obj_t *value);
static inline void obj_set_property(obj_t *o, tstr_t property, obj_t *value)
{
    _obj_set_property(o, property, obj_get(value));
}

void obj_set_property_str(obj_t *o, tstr_t property, tstr_t value);
void obj_set_property_int(obj_t *o, tstr_t property, int value);
int obj_get_property_int(int *value, obj_t *o, tstr_t property);
int obj_get_int(obj_t *o);
double obj_get_fp(obj_t *o);
tstr_t obj_get_str(obj_t *o);
void obj_inherit(obj_t *son, obj_t *parent);

/* General utility functions */
static inline int obj_eq(obj_t *a, obj_t *b)
{
    obj_t *eq;
    int ret;

    eq = obj_do_op(TOK_IS_EQ_STRICT, obj_get(a), obj_get(b));
    ret = obj_true(eq);
    obj_put(eq);
    return ret;
}

int throw_exception(obj_t **po, tstr_t *desc);

#endif
