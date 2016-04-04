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
#ifndef __PRINT_H__
#define __PRINT_H__

static FILE *fp;

#define _P(fmt, args...) fprintf(fp, fmt, ##args)
#define P(fmt, args...) fprintf(fp, fmt "\n", ##args)

static inline void print_replace(const char *str, char replaceme,
    const char *with)
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

static inline void __print_table_header(int n, const char *labels[])
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

static inline void __print_table_row(int n, const char *labels[])
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

#define print_section(fmt, args...) P("## " fmt, ##args)
#define print_subsection(fmt, args...) P("### " fmt, ##args)
#define print_subsubsection(fmt, args...) P("#### " fmt, ##args)

static inline void print_code_block(const char *code)
{
    _P("    ");
    print_replace(code, '\n', "\n    ");
    P("");
    P("");
}

#endif
