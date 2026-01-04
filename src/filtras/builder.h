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
#include "substitutions/subs.hpp"


class BuilderFT: public Filtra
{
    json payload_;
    json extra_;

public:
    BuilderFT(PipelineIface const & pi, json const & config)
            :Filtra(pi, config)
    {
        if( config.contains("payload") )
        {
            payload_ = config.at("payload");
        }

        if( config.contains("extra") )
        {
            extra_ = config.at("extra");
        }
    }

    string process_message( MessageWrapper &msg_w ) override
    {
        if( ! payload_.empty() )
        {
            json payload = payload_;
            build(payload, msg_w);
            msg_w.msg().get_json() = payload;
        }

        if( ! extra_.empty() )
        {
            json extra = extra_;
            build(extra, msg_w);

            auto & extra_storage = msg_w.get_extra();

            for( auto & [key, val] : extra.items() )
            {
                extra_storage.add_extra(key, val.dump());
            }
        }

        msg_w.pass();
        return "";
    }

    static pair<string, json> get_schema(){
        json schema = Filtra::get_schema();
        schema.merge_patch({
            {"encoder", {
                {"type", "string"},
                {"options", {"json"}},
                {"default", "json"},
                {"required", true}
            }},
            {"payload", {
                {"type", "object"},
                {"required", false}
            }},
            {"extra", {
                {"type", "object"},
                {"required", false}
            }}
        });
        return {"builder", schema};
    }

private:
    void build( json & output, MessageWrapper &msg_w )
    {
        auto se = SubsEngine(msg_w);

        if( output.is_object() )
        {
            substitute(se, output);
        }
        else if( output.is_string() )
        {
            output = substitute(se, output.get<string>());
        }
        else{
            throw std::runtime_error("Can not build!");
        }
    }

    void substitute(SubsEngine & se, json & j){
        for(auto it = j.begin(); it != j.end(); ++it){
            if(it->is_string()){
                auto result = se.substitute(it.value());
                if(std::holds_alternative<string>(result)){
                    it.value() = std::get<string>(result);
                }else{
                    it.value() = std::get<json>(result);
                }
            }else if(it->is_object() || it->is_array()){
                substitute(se, *it);
            }
        }
    }

    json substitute(SubsEngine & se, string const & s){
        auto result = se.substitute(s);
        if(std::holds_alternative<string>(result)){
            return std::get<string>(result);
        }else{
            return std::get<json>(result);
        }
    }
};


#endif  // __M2E_BRIDGE_BUILDER_FT_H__