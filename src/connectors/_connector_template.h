/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2025-2026 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_BRIDGE_<CONNECTOR_NAME>_CONNECTOR_H__
#define __M2E_BRIDGE_<CONNECTOR_NAME>_CONNECTOR_H__


#include "m2e_exceptions.h"
#include "m2e_message/message_wrapper.h"


class <CONNECTOR_NAME>Connector{
public:
    Connector(std::string pipeid, ConnectorMode mode, json const & config){
        if(authbundle_id_.empty()){
            throw std::runtime_error("authbundle_id cannot be null for email connector");
        }else{
            parse_authbundle();
        }

        try{
            polling_period_ = config.at("polling_period").get<unsigned>();
        }catch(json::exception){
            polling_period_ = 0;
        }
        // authbundle_id
        try{
            authbundle_id_ = config.at("authbundle_id").get<std::string>();
        }catch(json::exception){
            authbundle_id_ = "";
        }
    }

    static json get_schema(){
        json schema = {
            {"tags", {""}},
            {"modes", {"in", "out"}}
        };
        return schema;
    }

private:
    virtual Message const do_receive(){
        throw std::logic_error("do_receive not implemented!");
    }

    virtual void do_send(MessageWrapper & msg_w){
        throw std::logic_error("do_receive not implemented!");
    }

    virtual void do_connect(){}

    virtual void do_disconnect(){}

    virtual void do_stop(){}
};


#endif  // __M2E_BRIDGE_<CONNECTOR_NAME>_CONNECTOR_H__