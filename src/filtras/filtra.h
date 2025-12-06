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


#ifndef __M2E_BRIDGE_FILTRA_H__
#define __M2E_BRIDGE_FILTRA_H__


#include "m2e_aliases.h"
#include "pipeline_iface.h"
#include "m2e_message/message_wrapper.h"
#include "iostream"


class Filtra{
public:
    Filtra(PipelineIface const & pipeline, json const & config):name_(){
        name_ = config.value("name", "");

        std::string const & msg_format = config.value("msg_format", "raw");
        if(msg_format == "json"){
            msg_format_ = MessageFormat::Type::JSON;
        }else if(msg_format == "raw"){
            msg_format_ = MessageFormat::Type::RAW;
        }

        logical_negation_ = config.value("logical_negation", false);

        if(config.contains("queues")){
            json const & j_queues = config.at("queues");
            queue_ids_ = vector<string>(j_queues.begin(), j_queues.end());
        }

        if(config.contains("metadata")){
            metadata_ = config.at("metadata");
        }

        string goto_hop = config.value("goto", "");
        hops_.first = config.value("goto_passed", goto_hop);
        hops_.second = config.value("goto_rejected", goto_hop);
    }
    virtual ~Filtra(){}
    string process(MessageWrapper & msg_w){
        msg_w.add_metadata(metadata_);
        return process_message(msg_w);
    }
    Message process(){
        return generate_message();
    }
    vector<string> const & get_destinations(){return queue_ids_;}
    string const & get_name(){return name_;}
    hops_t const & get_hops(){return hops_;}

    virtual void start() {}
    virtual void stop() {}

    static json get_schema(){
        return json {
            {"name", {
                {"type", "string"},
                {"default", ""},
                {"required", false}
            }},
            {"msg_format", {
                {"type", "string"},
                {"options", {"json", "raw"}},
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
        };
    }

protected:
    virtual string process_message(MessageWrapper &msg_w) = 0;
    virtual Message generate_message(){return Message();}

    MessageFormat::Type msg_format_ {MessageFormat::Type::UNKN};
    bool logical_negation_ {false};
    vector<string> queue_ids_;

private:
    string name_;
    hops_t hops_;
    json metadata_;
};


#endif  // __M2E_BRIDGE_FILTRA_H__
