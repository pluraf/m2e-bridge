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
#include "internal_queue.h"


class InternalConnector: public Connector{
    string queuid_;
    InternalQueue & queue_;
    size_t buffer_size_ {0};
    RQueue incoming_;
public:
    InternalConnector(std::string pipeid, ConnectorMode mode, json const & config):
        Connector(pipeid, mode, config),
        queuid_(config.at("name")),
        queue_(InternalQueue::get_queue(queuid_))
    {
        buffer_size_ = config.value("buffer_size", 100);
    }

    void connect()override
    {
        if(mode_ == ConnectorMode::IN){
            incoming_ = queue_.subscribe(buffer_size_);
        }
    }

    void disconnect()override{
        if(mode_ == ConnectorMode::IN){
            queue_.unsubscribe(incoming_);
        }
    }

    Message const do_receive()override{
        return *incoming_.pop();
    }

    void do_send(MessageWrapper & msg_w)override{
        queue_.push(msg_w.msg_ptr());
    }

    void stop()override{
        Connector::stop();
    }
};


static const json internal_connector_schema_ = {
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