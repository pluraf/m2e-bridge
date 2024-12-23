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


#ifndef __M2E_BRIDGE_NOP_FT_H__
#define __M2E_BRIDGE_NOP_FT_H__


#include "filtra.h"


class NopFT:public Filtra{
public:
    NopFT(PipelineIface const & pi, json const & json_descr):Filtra(pi, json_descr){}
    string process_message(MessageWrapper &msg_w)override{
        msg_w.pass();
        return "";
    }
};


static const json nop_filtra_schema_ = {
    "nop", {
        {"type", {
            {"type", "string"},
            {"enum", {"nop"}},
            {"required", true}
        }},
        {"name", {
            {"type", "string"},
            {"default", ""},
            {"required", false}
        }},
        {"msg_format", {
            {"type", "string"},
            {"enum", {"json", "raw"}},
            {"default", "raw"},
            {"required", false}
        }},
        {"logical_negation", {
            {"type", "boolean"},
            {"default", false},
            {"required", false}
        }},
        {"queues", {
            {"type", "array"},
            {"items", {{"type", "string"}}},
            {"required", false}
        }},
        {"metadata", {
            {"type", "object"},
            {"required", false}
        }},
        {"goto", {
            {"type", "string"},
            {"default", ""},
            {"required", false}
        }},
        {"goto_passed", {
            {"type", "string"},
            {"default", ""},
            {"required", false}
        }},
        {"goto_rejected", {
            {"type", "string"},
            {"default", ""},
            {"required", false}
        }}
    }
};


#endif  // __M2E_BRIDGE_NOP_FT_H__