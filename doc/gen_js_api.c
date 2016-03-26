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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/tp_misc.h"
#include "doc/gen_js_api.h"
#include "version_data.h"

static FILE *fp;

#define CONST(...)
#define CONST_INT_VAL(...)
#define CATEGORY_INIT(...)

static doc_object_t global_env_desc = {
    .name = "global",
    .display_name = "Global",
};

#define OBJECT(disp_name, name, doc...) static doc_object_t name##_desc = doc;
#define PROTOTYPE(disp_name, name, doc...) static doc_object_t name##_desc = doc;
#define CLASS_PROTOTYPE(disp_name, name, proto, class, doc...) static doc_object_t name##_desc = doc;
#define FUNCTION(disp_name, parent, name, doc...) static doc_function_t parent##_##name##_desc = doc;
#define CONSTRUCTOR FUNCTION
#define CATEGORY(name, o, doc...) static doc_object_t name##_desc = doc;
#include "doc/doc.h"
#undef FUNCTION
#undef CONSTRUCTOR
#undef OBJECT
#undef CLASS_PROTOTYPE
#undef PROTOTYPE
#undef CATEGORY

static doc_object_t *objs[] = {
    &global_env_desc,
#define OBJECT(disp_name, name, doc...) &name##_desc,
#define PROTOTYPE(disp_name, name, doc...) &name##_desc,
#define CLASS_PROTOTYPE(disp_name, name, proto, class, doc...) &name##_desc,
#define CATEGORY(name, o, doc...) &name##_desc,
#define FUNCTION(...)
#define CONSTRUCTOR FUNCTION
#include "doc/doc.h"
#undef FUNCTION
#undef CONSTRUCTOR
#undef OBJECT
#undef CLASS_PROTOTYPE
#undef PROTOTYPE
#undef CATEGORY
    NULL
};

static doc_function_t *funcs[] = {
#define OBJECT(...)
#define PROTOTYPE(...)
#define CLASS_PROTOTYPE(...)
#define CATEGORY(...)
#define FUNCTION(disp_name, parent, name, doc...) &parent##_##name##_desc,
#define CONSTRUCTOR FUNCTION
#include "doc/doc.h"
#undef FUNCTION
#undef CONSTRUCTOR
#undef OBJECT
#undef CLASS_PROTOTYPE
#undef PROTOTYPE
#undef CATEGORY
    NULL
};

#define _P(fmt, args...) fprintf(fp, fmt, ##args)
#define P(fmt, args...) fprintf(fp, fmt "\n", ##args)

static doc_element_t main_de = { 
    .name = "index", 
    .display_name = "TinkerPal " TINKERPAL_VERSION " API" 
};

static void print_replace(const char *str, char replaceme, const char *with)
{
    int n = strlen(str);

    while (n--)
    {
        if (*str == replaceme)
            _P("%s", with);
        else
            _P("%c", *str);
        str++;
    }
}

static void __print_table_header(int n, const char *labels[])
{
    int i;

    for (i = 0; i < n; i++)
      _P("|%s", *labels++);
    P("|");
    for (i = 0; i < n; i++)
      _P("|---");
    P("|");
}
#define print_table_header(args...) \
    SPLAT(__print_table_header, const char *, args)

static void __print_table_row(int n, const char *labels[])
{
    int i;

    for (i = 0; i < n; i++)
    {
        _P("|");
        print_replace(*labels++, '\n', "<br>");
    }
    P("|");
}
#define print_table_row(args...) \
    SPLAT(__print_table_row, const char *, args)

#define print_section(fmt, args...) P("# " fmt, ##args)
#define print_subsection(fmt, args...) P("## " fmt, ##args)
#define print_subsubsection(fmt, args...) P("### " fmt, ##args)

static void print_code_block(const char *code)
{
    _P("    ");
    print_replace(code, '\n', "\n    ");
    P("");
    P("");
}

static void print_function_params(doc_function_t *f)
{
    const doc_function_param_t *p;

    if (!f->params->name)
        return;

    print_table_header("Parameter Name", "Description");
    for (p = f->params; p->name; p++)
        print_table_row(p->name, p->description);
}

static void print_object(doc_object_t *o)
{
    doc_function_t **f;

    print_section("%s", o->display_name);
    for (f = funcs; *f; f++)
    {
        if ((*f)->parent != o)
            continue;

        print_subsection("%s%s", (*f)->display_name,
            (*f)->flags & FUNCTION_FLAG_CONSTRUCTOR ? " (constructor)" : "");
        P("");
        P("%s", (*f)->description);
        print_subsubsection("Example");
        print_code_block((*f)->example);
        print_function_params(*f);
        P("");
    }
}

static int object_has_constructors(doc_object_t *o)
{
    doc_function_t **f;

    for (f = funcs; *f; f++)
    {
        if ((*f)->parent != o)
            continue;

        if ((*f)->flags & FUNCTION_FLAG_CONSTRUCTOR)
            return 1;
    }
    return 0;
}

static int object_has_methods(doc_object_t *o)
{
    doc_function_t **f;

    for (f = funcs; *f; f++)
    {
        if ((*f)->parent != o)
            continue;

        if (!((*f)->flags & FUNCTION_FLAG_CONSTRUCTOR))
            return 1;
    }
    return 0;
}

static int object_has_functions(doc_object_t *o)
{
    return object_has_constructors(o) || object_has_methods(o);
}

static void initialize(void)
{
#define OBJECT(disp_name, n, ...) do { \
    n##_desc.name = #n; \
    if (!n##_desc.display_name) \
        n##_desc.display_name = disp_name; \
    n##_desc.parent = &main_de; \
} while(0);
#define PROTOTYPE(disp_name, n, ...) do { \
    n##_desc.name = #n; \
    n##_desc.display_name = disp_name; \
    n##_desc.parent = &main_de; \
} while(0);
#define CLASS_PROTOTYPE(disp_name, n, proto, class, ...) do { \
    n##_desc.name = #n; \
    n##_desc.display_name = disp_name; \
    n##_desc.parent = &main_de; \
} while(0);
#define FUNCTION(disp_name, p, n, doc...) do { \
    p##_##n##_desc.name = #n; \
    p##_##n##_desc.display_name = disp_name; \
    p##_##n##_desc.parent = &p##_desc; \
} while(0);
#define CONSTRUCTOR(disp_name, p, n, doc...) do { \
    FUNCTION(disp_name, p, n, doc); \
    p##_##n##_desc.flags = FUNCTION_FLAG_CONSTRUCTOR; \
} while(0);
#define CATEGORY(n, o, ...) do { \
    n##_desc.name = #n; \
    n##_desc.parent = &main_de; \
} while(0);
#include "doc/doc.h"
#undef FUNCTION
#undef CONSTRUCTOR
#undef OBJECT
#undef CLASS_PROTOTYPE
#undef PROTOTYPE
#undef CATEGORY
}

static void print_header(void)
{
    P("---");
    P("title: API Guide | Tinkerpal " TINKERPAL_VERSION);
    P("markdown2extras: tables, wiki-tables, code-friendly");
    P("---");
    P("#API");
    P("Objects and methods detailed below");
    P("");
}

int main(int argc, char *argv[])
{
    doc_function_t **f;
    doc_object_t **o;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    initialize();

    if (!(fp = fopen(argv[1], "w")))
        exit(1);

    print_header();

    for (o = objs; *o; o++)
    {
        if (!object_has_functions(*o))
            continue;

        print_object(*o);
    }

    fclose(fp);
    return 0;
}
