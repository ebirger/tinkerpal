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
#include "util/tp_types.h"
#include "util/tstr_list.h"
#include "mem/tmalloc.h"

#ifdef CONFIG_OBJ_DOC
#include "doc/tdocs.h"
#endif

typedef struct var_t var_t;

#define Sprototype INTERNAL_S("prototype")

typedef struct {
    /* Trick : We use flags in obj_t for subclasses purposes - ugly, but
     *   saves space.
     */
#define OBJ_STATIC 0x01
#define OBJ_NUM_FP 0x02
    /* Use 'call' as a construct routine instead of the default
     * 'construct' function
     */
#define OBJ_FUNCTION_CONSTRUCTOR 0x04
    unsigned char flags;
    unsigned char class;
    short ref_count;
    var_t *properties;
} obj_t;

typedef int (*call_t)(obj_t **ret, obj_t *this, int argc, obj_t *argv[]);
typedef void (*code_free_cb_t)(void *code);

typedef struct {
    obj_t obj;
    call_t call;
    void *code;
    code_free_cb_t code_free_cb;
    obj_t *scope;
    tstr_list_t *formal_params;
#ifdef CONFIG_OBJ_DOC
    doc_function_t doc;
#endif
} function_t;

typedef struct {
    tstr_t *name;
    obj_t **parent;
    call_t call;
#ifdef CONFIG_OBJ_DOC
    doc_function_t doc;
    char *doc_name;
#endif
} function_template_t;

typedef struct {
    int argc;
    obj_t **argv;
} function_args_t;

typedef struct {
    obj_t obj;
    union {
        int i;
        double fp;
    } value;
} num_t;

/* Trick: use the LSB to mark if we are an 'immediate' integer value
 * or an obj pointer under the assumption that pointers are always
 * aligned
 */
#define OBJ_IS_INT_VAL(x) (((uint_ptr_t)x) & 0x1)

#define NUM_IS_FP(x) (!OBJ_IS_INT_VAL(x) && ((x)->obj.flags & OBJ_NUM_FP))
#define NUM_SET_FP(x) ((x)->obj.flags |= OBJ_NUM_FP)
#define INT_VAL(x) (((int_ptr_t)x)>>1)
#define NUM_INT(x) (OBJ_IS_INT_VAL(x) ? INT_VAL(x) : ((x)->value.i))
#define NUM_INT_SET(x, val) (x)->value.i = (val)
#define NUM_FP(x) ((x)->value.fp)

typedef struct {
    obj_t obj;
    tstr_t value;
} string_t;

typedef struct {
    obj_t obj;
    int is_true;
} bool_t;

typedef struct {
    obj_t obj;
} env_t;

typedef string_t array_buffer_t;

typedef struct {
    obj_t obj;
    array_buffer_t *array_buffer;
#define ABV_SHIFT_8_BIT 0
#define ABV_SHIFT_16_BIT 0x01
#define ABV_SHIFT_32_BIT 0x02
#define ABV_SHIFT_MASK 0x03
#define ABV_FLAG_UNSIGNED 0x04
    u32 flags;
    u32 offset; /* in view units */
    u32 length; /* in view units */
} array_buffer_view_t;

typedef struct {
    obj_t obj;
    function_args_t args;
} arguments_t;

typedef struct {
    obj_t obj;
    void *ptr;
    void (*free)(void *ptr);
} pointer_t;

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
#define ARRAY_BUFFER_CLASS 10
#define ARRAY_BUFFER_VIEW_CLASS 11
#define ARGUMENTS_CLASS 12
#define POINTER_CLASS 13
#define CLASS_LAST POINTER_CLASS
#define OBJ_CLASS(obj) (OBJ_IS_INT_VAL(obj) ? NUM_CLASS : (obj)->class)

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

#define STATIC_CONSTRUCTOR(f) { \
    .obj = _STATIC_OBJ(FUNCTION_CLASS, OBJ_FUNCTION_CONSTRUCTOR), \
    .call = f, .code = NULL, .scope = NULL, .formal_params = NULL }

#define STATIC_STRING(s) { .obj = STATIC_OBJ(STRING_CLASS), .value = S(s) }
#define STATIC_NUM(v) { .obj = STATIC_OBJ(NUM_CLASS), .value.i = v }
#define STATIC_NUM_FP(v) { .obj = _STATIC_OBJ(NUM_CLASS, OBJ_NUM_FP), \
    .value.fp = v }

/* Generic obj methods */
void obj_walk(obj_t *o, void (*cb)(obj_t *o));
obj_t *obj_cast(obj_t *o, unsigned char class);
obj_t **obj_var_create(obj_t *o, const tstr_t *str);
obj_t *obj_get_own_property(obj_t ***lval, obj_t *o, const tstr_t *str);
obj_t *obj_get_property(obj_t ***lval, obj_t *o, const tstr_t *property);
obj_t *obj_has_property(obj_t *o, const tstr_t *property); /* TRUE/FALSE */
obj_t *obj_do_op(token_type_t op, obj_t *oa, obj_t *ob);

static inline obj_t *obj_get(obj_t *o)
{
    if (!o)
        return NULL;

    if (OBJ_IS_INT_VAL(o))
        return o;

    o->ref_count++;
    return o;
}

void _obj_put(obj_t *o);

static inline void obj_put(obj_t *o)
{
    if (!o || o == UNDEF || OBJ_IS_INT_VAL(o))
        return;

    if (--o->ref_count > 0)
        return;

    _obj_put(o);
}

void obj_dump(printer_t *printer, obj_t *obj);
int obj_true(obj_t *o);
void obj_describe(printer_t *printer, obj_t *obj);

/* "num" objects methods */
obj_t *num_new_fp(double v);
obj_t *num_new_int(int v);
obj_t *num_new(tnum_t n);

static inline int is_num(obj_t *o)
{
    return o && OBJ_CLASS(o) == NUM_CLASS;
}

static inline num_t *to_num(obj_t *o)
{
    tp_assert(is_num(o));
    return (num_t *)o;
}

/* "function" objects methods */
obj_t *function_new(tstr_list_t *params, void *code, code_free_cb_t code_free,
    obj_t *scope, call_t call);
int function_call(obj_t **ret, obj_t *this_obj, int argc, obj_t *argv[]);
int function_call_construct(obj_t **ret, int argc, obj_t *argv[]);

static inline void function_args_init(function_args_t *args, obj_t *func)
{
    args->argc = 0;
    args->argv = tmalloc(1 + CONFIG_MAX_FUNCTION_CALL_ARGS * sizeof(obj_t *),
        "Args");
    args->argv[args->argc++] = func; /* argv[0] is our very own function */
}

static inline int function_args_add(function_args_t *args, obj_t *obj)
{
    if (args->argc == CONFIG_MAX_FUNCTION_CALL_ARGS - 1)
    {
        tp_err("Exceeded maximal function call arguments.\n"
            "You can refine this behavior by increasing "
            "CONFIG_MAX_FUNCTION_CALL_ARGS\n");
        return -1;
    }
    args->argv[args->argc++] = obj;
    return 0;
}

static inline void function_args_uninit(function_args_t *args)
{
    tfree(args->argv);
}

static inline int is_function(obj_t *o)
{
    return o && OBJ_CLASS(o) == FUNCTION_CLASS;
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
    return o && OBJ_CLASS(o) == STRING_CLASS;
}

static inline string_t *to_string(obj_t *o)
{
    tp_assert(is_string(o));
    return (string_t *)o;
}

/* "object" objects methods */
obj_t *object_new(void);

static inline int is_object(obj_t *o)
{
    return o && OBJ_CLASS(o) == OBJECT_CLASS;
}

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
    return o && OBJ_CLASS(o) == ENV_CLASS;
}

/* "array" objects methods */
obj_t *array_new(void);
obj_t *array_push(obj_t *arr, obj_t *item);
obj_t *array_pop(obj_t *arr);
void array_length_set(obj_t *arr, int length);
int array_length_get(obj_t *arr); /* Works on abstract arrays as well */
obj_t *array_lookup(obj_t *arr, int index);
#define _array_set_item(arr, idx, item) _obj_set_int_property(arr, idx, item)

static inline int is_array(obj_t *o)
{
    return o && OBJ_CLASS(o) == ARRAY_CLASS;
}

typedef struct {
    obj_t *arr; /* Iterated array */
    obj_t *obj; /* The current iterated item */
    int k; /* Index of the current iterated item */
    int len; /* Array length */
#define ARRAY_ITER_FLAG_REVERSE 0x01 /* Iterate from length-1 to 0 */
#define ARRAY_ITER_FLAG_INCLUDE_EMPTY 0x02 /* Iterate over empty cells */
    u8 flags;
} array_iter_t;

void array_iter_init(array_iter_t *iter, obj_t *arr, u8 flags);
/* Returns 0 upon on the last element */
int array_iter_next(array_iter_t *iter);
void array_iter_uninit(array_iter_t *iter);

/* typed arrays objects methods */
obj_t *array_buffer_new(int length);
obj_t *array_buffer_view_new(obj_t *array_buffer, u32 flags, u32 offset, 
    int length);

static inline int is_array_buffer(obj_t *o)
{
    return o && OBJ_CLASS(o) == ARRAY_BUFFER_CLASS;
}

static inline array_buffer_t *to_array_buffer(obj_t *o)
{
    tp_assert(is_array_buffer(o));
    return (array_buffer_t *)o;
}

/* Note: no bounds checking on array_buffer_view_item_val_get() and
 * array_buffer_view_item_val_set()
 */
int array_buffer_view_item_val_get(array_buffer_view_t *v, int idx);
int array_buffer_view_item_val_set(array_buffer_view_t *v, int idx, int val);

static inline int is_array_buffer_view(obj_t *o)
{
    return o && OBJ_CLASS(o) == ARRAY_BUFFER_VIEW_CLASS;
}

static inline array_buffer_view_t *to_array_buffer_view(obj_t *o)
{
    tp_assert(is_array_buffer_view(o));
    return (array_buffer_view_t *)o;
}

/* "arguments" objects methods */
obj_t *arguments_new(function_args_t *args);

static inline int is_arguments(obj_t *o)
{
    return o && OBJ_CLASS(o) == ARGUMENTS_CLASS;
}

static inline arguments_t *to_arguments(obj_t *o)
{
    tp_assert(is_arguments(o));
    return (arguments_t *)o;
}

/* "pointer" objects methods */
obj_t *pointer_new(void *ptr, void (*free)(void *ptr));

static inline int is_pointer(obj_t *o)
{
    return o && OBJ_CLASS(o) == POINTER_CLASS;
}

static inline pointer_t *to_pointer(obj_t *o)
{
    tp_assert(is_pointer(o));
    return (pointer_t *)o;
}

/* Initialization sequence functions */
void obj_class_set_prototype(unsigned char class, obj_t *proto);

/* Utility functions for obj property manipulation */
void _obj_set_property(obj_t *o, tstr_t property, obj_t *value);
static inline void obj_set_property(obj_t *o, tstr_t property, obj_t *value)
{
    _obj_set_property(o, property, obj_get(value));
}

static inline void obj_set_property_str(obj_t *o, tstr_t property, tstr_t value)
{
    _obj_set_property(o, property, string_new(value));
}

static inline void obj_set_property_int(obj_t *o, tstr_t property, int value)
{
    _obj_set_property(o, property, num_new_int(value));
}

static inline void obj_set_property_fp(obj_t *o, tstr_t property, double value)
{
    _obj_set_property(o, property, num_new_fp(value));
}

static inline void _obj_set_int_property(obj_t *o, int property, obj_t *value)
{
    tstr_t s = int_to_tstr(property);

    _obj_set_property(o, s, value);
    tstr_free(&s);
}

static inline void obj_set_int_property(obj_t *o, int property, obj_t *value)
{
    _obj_set_int_property(o, property, obj_get(value));
}

int obj_get_property_int(int *value, obj_t *o, const tstr_t *property);
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

void js_obj_foreach_alloced_obj(void (*cb)(void *obj));
void js_obj_uninit(void);
void js_obj_init(void);

#endif
