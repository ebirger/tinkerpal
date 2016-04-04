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
#include "util/debug.h"
#include "js/js_builtins.h"
#include "js/js_obj.h"

extern obj_t *global_env;

#define FUNCTION(n, o, f, ...) \
    extern int f(obj_t **ret, obj_t *this, int argc, obj_t *argv[]);
#define CONSTRUCTOR(n, o, f, ...) \
    extern int f(obj_t **ret, obj_t *this, int argc, obj_t *argv[]); \
    static function_t f##_func = STATIC_CONSTRUCTOR(f);
#define OBJECT(n, o, ...) static obj_t o##_obj = STATIC_OBJ(OBJECT_CLASS), *o = &o##_obj;
#define CONST(n, o, c, v) static num_t o##c = v;
#define CONST_INT_VAL(n, o, c, v) static obj_t *o##c = (obj_t *)(((v)<<1)| 0x1);
#define CLASS_PROTOTYPE(n, o, p, c...) OBJECT(n, o)
#define PROTOTYPE(n, o, ...) OBJECT(n, o)
#define CATEGORY(n, o, ...) static obj_t *n;
#define CATEGORY_INIT(init, uninit, ...) \
    extern void init(void); \
    extern void uninit(void);

#include "jsapi.h"

#undef FUNCTION
#undef OBJECT
#undef CONSTRUCTOR
#undef CLASS_PROTOTYPE
#undef PROTOTYPE
#undef CONST
#undef CONST_INT_VAL
#undef CATEGORY
#undef CATEGORY_INIT

/* Function templates array */
#define CONSTRUCTOR(...)
#define OBJECT(...)
#define PROTOTYPE(...)
#define CONST(...)
#define CONST_INT_VAL(...)
#define CATEGORY(...)
#define CLASS_PROTOTYPE(n, o, p, c, ...)
#define CATEGORY_INIT(init, uninit, ...)
#ifdef CONFIG_OBJ_DOC
#define FUNCTION(n, o, f, d...) \
    { \
        .name = &S(n), \
        .parent = &o, \
        .call = f, \
        .doc_name = n, \
        .doc = d \
    },
#else
#define FUNCTION(n, o, f, ...) \
    { \
        .name = &S(n), \
        .parent = &o, \
        .call = f \
    },
#endif

const function_template_t function_templates[] = {
#include "jsapi.h"
    {}
};

#undef FUNCTION
#undef OBJECT
#undef CONSTRUCTOR
#undef CLASS_PROTOTYPE
#undef PROTOTYPE
#undef CONST
#undef CONST_INT_VAL
#undef CATEGORY
#undef CATEGORY_INIT

void js_builtins_uninit(void)
{
#define FUNCTION(...)
#define CONSTRUCTOR(...)
#define OBJECT(...)
#define PROTOTYPE(...)
#define CONST(...)
#define CONST_INT_VAL(...)
#define CATEGORY(...)
#define CLASS_PROTOTYPE(n, o, p, c, ...) \
    obj_class_set_prototype(c, NULL); \
    obj_put(o);
#define CATEGORY_INIT(init, uninit, ...) \
    uninit();

#include "jsapi.h"

#undef FUNCTION
#undef OBJECT
#undef CONSTRUCTOR
#undef CLASS_PROTOTYPE
#undef PROTOTYPE
#undef CONST
#undef CONST_INT_VAL
#undef CATEGORY
#undef CATEGORY_INIT
}

void js_builtins_init(void)
{
#ifdef CONFIG_OBJ_DOC
#define OBJ_DOC_FUNCTION_INIT(n, o, f, d...) do { \
    doc_function_t doc = d; \
    memcpy(&f##_func.doc, &doc, sizeof(doc_function_t)); \
    f##_func.doc.name = n; \
} while(0)
#else
#define OBJ_DOC_FUNCTION_INIT(n, o, f, d...)
#endif

#define FUNCTION(n, o, f, d...)
#define CONSTRUCTOR(n, o, f, d...) do { \
    _obj_set_property(global_env, S(n), (obj_t *)&f##_func); \
    OBJ_DOC_FUNCTION_INIT(n, global_env, f, d); \
    _obj_set_property(&f##_func.obj, Sprototype, o); \
} while(0);
#define OBJECT(n, o, ...) do { \
    tstr_t oname = S(n); \
    _obj_set_property(global_env, oname, o); \
} while(0);
#define CLASS_PROTOTYPE(n, o, p, c, ...) do { \
    _obj_set_property(o, Sprototype, p); \
    obj_class_set_prototype(c, o); \
} while(0);
#define CONST(n, o, c, v, ...) do { \
    tstr_t cname = S(n); \
    _obj_set_property(o, cname, (obj_t *)&o##c); \
} while(0);
#define CONST_INT_VAL(n, o, c, v, ...) do { \
    tstr_t cname = S(n); \
    _obj_set_property(o, cname, o##c); \
} while(0);
#define PROTOTYPE(...)
#define CATEGORY(n, o, ...) n = o;
#define CATEGORY_INIT(init, uninit, ...) \
    init();

#include "jsapi.h"

#undef FUNCTION
#undef OBJECT
#undef CONSTRUCTOR
#undef CLASS_PROTOTYPE
#undef PROTOTYPE
#undef CONST
#undef CONST_INT_VAL
#undef CATEGORY
#undef CATEGORY_INIT
}
