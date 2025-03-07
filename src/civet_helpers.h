/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


#ifndef __M2E_BRIDGE_CIVET_HELPERS_H__
#define __M2E_BRIDGE_CIVET_HELPERS_H__


#include <cstring>

#include "CivetServer.h"


inline char const * cv_get_last_segment(mg_request_info const * req_info)
{
    char const * last_segment = strrchr(req_info->request_uri, '/');
    if(last_segment && strlen(last_segment) > 1) return last_segment + 1;
    return NULL;
}


inline char const * cv_get_bearer_token(mg_connection * const conn)
{
    char const * auth_token = mg_get_header(conn, "Authorization");
    if (auth_token != NULL && strlen(auth_token) > 7) return auth_token + 7;  // Skip "Bearer "
    return NULL;
}


#endif  // __M2E_BRIDGE_CIVET_HELPERS_H__