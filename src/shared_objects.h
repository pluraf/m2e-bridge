/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2025 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_SHARED_OBJECTS_H__
#define __M2E_SHARED_OBJECTS_H__


#include "m2e_aliases.h"


class SharedObjects{
    json json_shared_objects_;
    map<string, byte*> memory_shared_objects_;

    SharedObjects() {}
public:
    SharedObjects(SharedObjects const &) = delete;
    SharedObjects & operator=(SharedObjects const &) = delete;

    static SharedObjects & get_instance(){
        static SharedObjects instance;
        return instance;
    }

    void init(json const & config);
    static json & get_json(string const & name);
    static json const & get_json();
    static byte * get_memory(string const & name);
};


#endif  // __M2E_SHARED_OBJECTS_H__