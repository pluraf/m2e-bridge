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


#ifndef __M2E_BRIDGE_BUILDER_FT_H__
#define __M2E_BRIDGE_BUILDER_FT_H__


#include "filtra.h"


class BuilderFT:public Filtra{
public:
    BuilderFT(PipelineIface const & pi, json const & json_descr):
            Filtra(pi, json_descr){
        payload_ = json_descr.at("payload");
    }

    string process_message(MessageWrapper &msg_w)override{
        if(msg_format_ == MessageFormat::JSON){
            msg_w.msg().get_json() = payload_;
        }else{
            throw std::runtime_error("Builder: Unknown encoder type");
        }
        msg_w.pass();
        return "";
    }

private:
    json payload_;
};


static const json builder_filtra_schema_ = {
    "builder", {
        {"type", {
            {"type", "string"},
            {"enum", {"builder"}},
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
        }},
        {"payload", {
            {"type", "object"},
            {"required", true}
        }}
    }
};


#endif  // __M2E_BRIDGE_BUILDER_FT_H__