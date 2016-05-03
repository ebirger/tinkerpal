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
#ifndef FUNCTION
#define FUNCTION(...)
#endif
#ifndef CONSTRUCTOR
#define CONSTRUCTOR(...)
#endif
#ifndef OBJECT
#define OBJECT(...)
#endif
#ifndef CONST
#define CONST(...)
#endif
#ifndef CONST_INT_VAL
#define CONST_INT_VAL(...)
#endif
#ifndef CLASS_PROTOTYPE
#define CLASS_PROTOTYPE(...)
#endif
#ifndef PROTOTYPE
#define PROTOTYPE(...)
#endif
#ifndef CATEGORY
#define CATEGORY(...)
#endif
#ifndef CATEGORY_INIT
#define CATEGORY_INIT(...)
#endif

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
