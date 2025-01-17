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


#ifndef __M2E_BRIDGE_CONNECTOR_H__
#define __M2E_BRIDGE_CONNECTOR_H__

#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

#include "m2e_message/message_wrapper.h"


enum class ConnectorMode {
    UNKN,
    IN,
    OUT
};


struct ConnectorStat{
    time_t last_in;
    time_t last_out;
    unsigned long count_in;
    unsigned long count_out;
};


class Connector{

protected:
    ConnectorMode mode_ {ConnectorMode::UNKN};
    std::string pipeid_;
    bool is_active_ {false};

public:
    Connector(std::string pipeid, ConnectorMode mode, json const & config){
        mode_ = mode;
        pipeid_ = pipeid;
        is_active_ = true;
    }
    virtual void connect(){}
    virtual void disconnect(){}
    virtual void stop(){is_active_=false;}
    Message receive(){
        Message msg = do_receive();
        ++stat_.count_in;
        auto now = chrono::system_clock::now().time_since_epoch();
        stat_.last_in = chrono::duration_cast<chrono::seconds>(now).count();
        return msg;
    }
    void send(MessageWrapper & msg_w){
        do_send(msg_w);
        ++stat_.count_out;
        auto now = chrono::system_clock::now().time_since_epoch();
        stat_.last_out = chrono::duration_cast<chrono::seconds>(now).count();
    }
    ConnectorStat get_statistics(){
        return stat_;
    }

protected:
    virtual Message do_receive(){
        throw std::runtime_error("do_receive not implemented");
    }
    virtual void do_send(MessageWrapper & msg_w){
        throw std::runtime_error("do_receive not implemented");
    }

private:
    ConnectorStat stat_ {0};
};


#endif  // __M2E_BRIDGE_CONNECTOR_H__