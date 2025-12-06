/* SPDX-License-Identifier: BSD-3-Clause */

/*
Copyright (c) 2025 Pluraf Embedded AB <code@pluraf.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
            {"authbundle_id", {
                {"type", "string"},
                {"options", {
                    {"url", "api/authbundle/"},
                    {"key", "authbundle_id"},
                }},
                {"required", false},
            }}
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