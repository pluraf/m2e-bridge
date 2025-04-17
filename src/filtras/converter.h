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


#ifndef __M2E_BRIDGE_CONVERTER_FT_H__
#define __M2E_BRIDGE_CONVERTER_FT_H__


#include "filtra.h"


class ConverterFT:public Filtra{
public:
    ConverterFT(PipelineIface const & pi, json const & json_descr):
            Filtra(pi, json_descr){
    }

    string process_message(MessageWrapper &msg_w)override{
        Message msg = msg_w.orig();
        string raw = msg.get_raw();
        uint16_t val = static_cast<uint8_t>(raw[0]) << 8 | static_cast<uint8_t>(raw[1]);
        msg_w.msg().get_raw() = std::to_string(static_cast<double>(val) / 10);
        msg_w.pass();
        return "";
    }

    static pair<string, json> get_schema(){
        json schema = Filtra::get_schema();
        schema.merge_patch({
            {"keys", {
                {"type", "array"},
                {"items", {{"type", "string"}}},
                {"required", true}
            }}
        });
        return {"eraser", schema};
    }

private:
    vector<string> keys_;
};


#endif  // __M2E_BRIDGE_CONVERTER_FT_H__