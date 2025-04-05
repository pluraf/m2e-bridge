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


#ifndef __M2E_BRIDGE_GENERATOR_CONNECTOR_H__
#define __M2E_BRIDGE_GENERATOR_CONNECTOR_H__


#include <thread>

#include "connector.h"


class GeneratorConnector: public Connector{
    unsigned long period_;  // milliseconds
    string payload_;
    bool first_call_ {true};
public:
    GeneratorConnector(std::string pipeid, ConnectorMode mode_, json const& json_descr)
        :Connector(pipeid, mode_, json_descr)
    {
        if(mode_ == ConnectorMode::OUT){
            throw configuration_error("GeneratorConnector does not support OUT mode!");
        }
        payload_ = json_descr["payload"];
        period_ = json_descr["period"];
    }

    Message const do_receive()override{
        if(first_call_){
            first_call_ = false;
            return Message(payload_);
        }
        unsigned long cnt = period_;
        while(is_active_){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (--cnt == 0) return Message(payload_);
        }
        throw std::underflow_error("No messages");
    }

    static pair<string, json> get_schema(){
        json schema = {
            {"period", {
                {"type", "integer"},
                {"required", true}
            }},
            {"payload", {
                {"type", "string"},
                {"required", true}
            }}
        };
        return {"generator", schema};
    }
};


#endif  // __M2E_BRIDGE_GENERATOR_CONNECTOR_H__