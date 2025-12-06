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


#ifndef __M2E_BRIDGE_EXTRACTOR_FT_H__
#define __M2E_BRIDGE_EXTRACTOR_FT_H__


#include <sstream>

#include "filtra.h"


class ExtractorFT:public Filtra{
public:
    ExtractorFT(PipelineIface const & pi, json const & json_descr):
            Filtra(pi, json_descr){
        for(auto const & [key, value] : json_descr["map"].items()){
            map_.push_back({key, vector<string>(value.begin(), value.end())});
        }
    }

    string process_message(MessageWrapper & msg_w)override{
        json output = json::object();
        if(msg_format_ == MessageFormat::Type::JSON){
            json const & j_payload = msg_w.msg().get_json();
            for(auto const & [name, key] : map_){
                try{
                    json const * payload = & j_payload;
                    for(auto const & subkey : key){
                        payload = & payload->at(subkey);
                    }
                    output[name] = std::move(* payload);
                }catch(std::exception){
                    continue;
                }
            }
            if(output.empty()){
                msg_w.reject();
            }else{
                msg_w.set_message(Message(output, MessageFormat::Type::JSON));
                msg_w.pass();
            }
        }
        return "";
    }

    static pair<string, json> get_schema(){
        json schema = Filtra::get_schema();
        schema.merge_patch({
            {"map", {
                {"type", "object"},
                {"required", true}
            }},
            {"msg_format", {
                {"type", "string"},
                {"options", {"json"}},
                {"default", "json"},
                {"required", true}
            }},
        });
        return {"extractor", schema};
    }

private:
    vector<pair<string, vector<string>>> map_;
};


#endif  // __M2E_BRIDGE_EXTRACTOR_FT_H__