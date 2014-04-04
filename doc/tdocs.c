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
#include "doc/tdocs.h"
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

#define BS_CSS "http://tinkerpal.org/wp-content/themes/" \
    "tinkjs/css/bootstrapwp.css?ver=0.90"

#define PRETTIFY_JS "https://google-code-prettify.googlecode.com/svn/" \
    "loader/run_prettify.js?lang=css&skin=desert"

#define JQUERY_JS "http://code.jquery.com/jquery-1.9.1.min.js"

typedef void (*body_function_t)(void *ctx);

static doc_element_t main_de = { 
    .name = "index", 
    .display_name = "TinkerPal " TINKERPAL_VERSION " API" 
};

static int level;

static char *function_file_path(doc_function_t *f)
{
    static char file_path[256];

    snprintf(file_path, sizeof(file_path), "%s%s", f->display_name,
        f->flags & FUNCTION_FLAG_CONSTRUCTOR ? "_constructor" : "");
    return file_path;
}

static void print_breadcrumb(doc_element_t **e)
{
    doc_element_t *eprev = NULL;
    char path[256] = {'\0'};
    int i = 0;

    P("<ul class='breadcrumb'>");

    for (; *e; e++, i++)
    {
        if (eprev)
        {
            int l = level;

            P("  <li>");
            _P("   <a href='");
            while (l--)
                _P("../");
            if (i != 1)
            {
                snprintf(path, sizeof(path), "%s%s%s/", path, *path ? "/" : "",
                    eprev->name);
            }
            P("%s%s.html'>%s</a>", path, eprev->name, eprev->display_name);
            P("    <span class='divider'>/</span>");
            P("  </li>");
        }
        eprev = *e;
    }
    P("  <li class='active'>");
    P("    %s", eprev->display_name);
    P("  </li>");
    P("</ul>");
}

static void print_function_params(doc_function_t *f)
{
    const doc_function_param_t *p;

    if (!f->params->name)
        return;

    P("<h2>Params</h2>");
    P("<table class='table'>");
    P("  <thead>");
    P("    <tr>");
    P("      <th>Parameter name</th>");
    P("      <th>Description</th>");
    P("    </tr>");
    P("  </thead>");
    P("  <tbody>");
    for (p = f->params; p->name; p++)
    {
        P("    <tr>");
        P("      <td>");
        P("        <strong>%s</strong><br>", p->name);
        P("      </td>");
        P("      <td>");
        P("        <p>%s</p>", p->description);
        P("      </td>");
        P("    </tr>");
    }
    P("  </tbody>");
    P("</table>");
}

static void print_function(void *ctx)
{
    doc_function_t *f = ctx;
    doc_element_t *bc[] = { &main_de, (doc_element_t *)f->parent, 
        (doc_element_t *)f, NULL };
    const doc_function_param_t *p;
    int is_top = f->parent == &global_env_desc;

    level = 1;
    print_breadcrumb(bc);

    P("<div>");
    P("  <div class='page-header'>");
    P("    <h1>");
    _P("      %s%s%s(", !is_top ? f->parent->display_name : "", 
        !is_top ? "." : "", f->display_name);
    for (p = f->params; p->name; p++)
        _P("%s%s", p == f->params ? "" : ", ", p->name);
    _P(")<br>");
    P("      <small>%s</small>", f->description);
    P("    </h1>");
    P("  </div>");
    P("<div>");
    P("<h2>Example</h2>");
    P("<pre class=\"prettyprint\">%s</pre>", f->example);
    print_function_params(f);
    P("<h2>Return Value</h2>");
    P("%s", f->return_value);
}

static void print_object(void *ctx)
{
    doc_function_t **f;
    doc_object_t *o = ctx;
    doc_element_t *bc[] = { &main_de, (doc_element_t *)o, NULL };

    level = 1;
    print_breadcrumb(bc);

    P("<div>");
    P("  <div class='page-header'>");
    P("    <h1>");
    P("      %s<br>", o->display_name);
    P("      <small></small>");
    P("    </h1>");
    P("  </div>");
    P("</div>");
    P("<div class='accordion' id='accordion'>");
    for (f = funcs; *f; f++)
    {
        if ((*f)->parent != o)
            continue;

        P("  <hr>");
        P("  <div class='pull-right small'>");
        P("    <a href='./%s.html'> >>> </a>", function_file_path(*f));
        P("  </div>");
        P("  <div>");
        P("    <h2>");
        P("      <a href='#description-index'");
        P("        class='accordion-toggle'");
        P("        data-toggle='collapse'");
        P("        data-parent='#accordion'>");
        P("         %s", (*f)->display_name);
        P("      </a><br>");
        P("      <small>%s</small>", (*f)->description);
        P("    </h2>");
        P("  </div>");
        P("  <div id='description-index' class='collapse accordion-body'>");
        P("    <h3>Examples</h3>");
        P("    <pre class=\"prettyprint\">%s</pre>", (*f)->example);
        print_function_params(*f);
        P("  </div>");
    }
    P("</div>");
}

static void print_head(void)
{
    P("<head>");
    P("  <title>API documentation</title>");
    P("  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>");
    P("  <link type='text/css' rel='stylesheet' href='%s'/>", BS_CSS);
    P("  <!-- IE6-8 support of HTML5 elements -->");
    P("  <!--[if lt IE 9]>");
    P("    <script src=\"http://html5shim.googlecode.com/svn/trunk/html5.js\"></script>");
    P("  <![endif]-->");
    P("</head>");
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

static void print_functions_table(doc_object_t *o)
{
    doc_function_t **f;

    if (object_has_constructors(o))
    {
        P("<table class='table'>");
        P("  <thead>");
        P("    <tr>");
        P("      <th>Constructor</th>");
        P("    </tr>");
        P("  </thead>");
        P("  <tbody>");
        for (f = funcs; *f; f++)
        {
            if ((*f)->parent != o)
                continue;

            if (!((*f)->flags & FUNCTION_FLAG_CONSTRUCTOR))
                continue;

            P("    <tr><td><a href='./%s/%s.html'>%s()</a></td></tr>", o->name, 
                    function_file_path(*f), (*f)->display_name);
        }
        P("  </tbody>");
        P("</table>");
    }
    if (object_has_methods(o))
    {
        P("<table class='table'>");
        P("  <thead>");
        P("    <tr>");
        P("      <th>Method</th>");
        P("      <th>Description</th>");
        P("    </tr>");
        P("  </thead>");
        P("  <tbody>");
        for (f = funcs; *f; f++)
        {
            if ((*f)->parent != o)
                continue;

            if (((*f)->flags & FUNCTION_FLAG_CONSTRUCTOR))
                continue;

            P("    <tr>");
            P("      <td><a href='./%s/%s.html'>%s()</a></td>", o->name, 
                function_file_path(*f), (*f)->display_name);
            P("      <td width='60%%'>%s</td>", (*f)->description);
            P("    </tr>");
        }
        P("  </tbody>");
        P("</table>");
    }
}

static void print_objects_table(void)
{
    doc_object_t **o;

    for (o = objs; *o; o++)
    {
        if (!object_has_functions(*o))
            continue;

        P("<h2>");
        P("  <a href='./%s/%s.html'>%s</a></br>", (*o)->name, (*o)->name,
            (*o)->display_name);
        P("  <small></small>");
        P("</h2>");

        print_functions_table(*o);
    }
}

static void print_index_body(void *dummy)
{
    doc_element_t *bc[] = { &main_de, NULL };

    print_breadcrumb(bc);

    P("<div>API description</div>");
    P("<h1 class='page-header'>Functions</h1>");

    print_objects_table();
}

static void print_body(body_function_t print, void *ctx)
{
    P("<body>");
    P("  <div class=\"container\">");
    P("    <div class=\"row\">");
    P("      <div id='container'>");

    print(ctx);

    P("      </div>");
    P("    </div>");
    P("    <hr>");
    P("    <footer>");
    P("    </footer>");
    P("  </div>");
    P("  <script type='text/javascript' src='%s'></script>", JQUERY_JS);
    P("<script type='text/javascript' src='%s'></script>", PRETTIFY_JS);
    P("</body>");
}

static void print_html(body_function_t print, void *ctx)
{
    P("<!DOCTYPE html>");
    P("<html>");
    print_head();
    print_body(print, ctx);
    P("</html>");
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

static void open_file(const char *folder, const char *file)
{
    char buf[256];

    if (fp)
        fclose(fp);

    if (folder)
    {
        sprintf(buf, "mkdir -p %s/%s", getenv("DOCS_DIR"), folder);
        system(buf);
    }

    sprintf(buf, "%s/%s%s%s.html", getenv("DOCS_DIR"), folder ? : "", 
        folder ? "/" : "", file);
    if (!(fp = fopen(buf, "w")))
        exit(1);
}

int main(void)
{
    doc_function_t **f;
    doc_object_t **o;

    initialize();

    open_file(NULL, "index");

    print_html(print_index_body, NULL);

    for (o = objs; *o; o++)
    {
        if (!object_has_functions(*o))
            continue;

        open_file((*o)->name, (*o)->name);
        print_html(print_object, (void *)*o);
    }

    for (f = funcs; *f; f++)
    {
        open_file((*f)->parent->name, function_file_path(*f));
        print_html(print_function, (void *)*f);
    }

    fclose(fp);
    return 0;
}
