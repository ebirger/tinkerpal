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
#include "main/console.h"
#include "js/js_types.h"
#include "js/js_obj.h"
#include "js/jsapi_decl.h"
#include "drivers/mmc/mmc.h"
#include "boards/board.h"

int do_mmc_constructor(obj_t **ret, obj_t *this, int argc, obj_t *argv[])
{
    mmc_params_t params;
    const mmc_params_t *p;

    /* XXX: would preferebly receive an object with optional hookup info */
    if (argc == 1)
    {
        tp_info("Using default hookup info\n");

        p = &board.mmc_params;
    }
    else if (argc != 4)
        return COMPLETION_THROW;
    else
    {
        params.spi_port = obj_get_int(argv[1]);
        params.mosi = obj_get_int(argv[2]);
        params.cs = obj_get_int(argv[3]);
        p = &params;
    }

    mmc_init(p);
    *ret = object_new();
    return 0;
}
