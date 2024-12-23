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


#ifndef __M2E_BRIDGE_THROTTLE_FT_H__
#define __M2E_BRIDGE_THROTTLE_FT_H__


#include "filtra.h"


class ThrottleFT:public Filtra{
public:
    ThrottleFT(PipelineIface const & pi, json const & json_descr):Filtra(pi, json_descr){
        rate_ = json_descr["rate"];
        delay_ms_ = 1000/rate_;
    }

    string process_message(MessageWrapper &msg_w)override{
        auto now = chrono::steady_clock::now();

        auto elapsed = chrono::duration_cast<std::chrono::milliseconds>(now - last_msg_time_).count();

        if(elapsed >= delay_ms_){
            last_msg_time_ = now;
            msg_w.pass();
            return "";
        }else{
            msg_w.reject();
            return "";
        }
    }

private:
    double rate_;  // Per seconds
    double delay_ms_;
    chrono::steady_clock::time_point last_msg_time_ {};
};


json throttle_filtra_schema_ = {
    "throttle", {
        {"type", {
            {"type", "string"},
            {"enum", {"throttle"}},
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
        {"rate", {
            {"type", "integer"},
            {"required", true}
        }}
    }
};


#endif  // __M2E_BRIDGE_THROTTLE_FT_H__