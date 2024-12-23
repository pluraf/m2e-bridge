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


#ifndef __M2E_BRIDGE_INTERNAL_CONNECTOR_H__
#define __M2E_BRIDGE_INTERNAL_CONNECTOR_H__


#include "connector.h"
#include "internal_queues.h"


class InternalConnector: public Connector{
public:
    InternalConnector(std::string pipeid, ConnectorMode mode, json const & config):
            Connector(pipeid, mode, config){
        queuid_ = config.at("name");
    }

    Message do_receive()override{
        TSQueue<Message> & q = InternalQueues::get_queue(queuid_);
        auto el = q.pop();
        while(is_active_ && ! el){
            q.wait();
            el = q.pop();
        }
        if(! el){
            throw std::out_of_range("");
        }
        return * el;
    }

    void do_send(MessageWrapper & msg_w)override{
        TSQueue<Message> & q = InternalQueues::get_queue(queuid_);
        q.push(msg_w.msg());
    }

    void stop()override{
        is_active_ = false;
        TSQueue<Message> & q = InternalQueues::get_queue(queuid_);
        q.exit_blocking_calls();
    }

private:
    string queuid_;
};


json internal_connector_schema_ = {
    "queue", {
        {"type", {
            {"type", "string"},
            {"enum", {"queue"}},
            {"required", true}
        }},
        {"authbundle_id", {
            {"type", "string"},
            {"required", true}
        }},
        {"name", {
            {"type", "string"},
            {"required", true}
        }}
    }
};


#endif  // __M2E_BRIDGE_INTERNAL_CONNECTOR_H__