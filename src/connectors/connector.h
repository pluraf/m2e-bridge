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
#include <thread>

#include "m2e_exceptions.h"
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
    ConnectorStat stat_ {0};
protected:
    ConnectorMode mode_ {ConnectorMode::UNKN};
    std::string pipeid_;
    bool is_active_ {false};
    time_t polling_period_ {0};
    string authbundle_id_;
    MessageFormat::Type msg_format_ {MessageFormat::Type::UNKN};
public:
    Connector(std::string pipeid, ConnectorMode mode, json const & config){
        mode_ = mode;
        pipeid_ = pipeid;
        is_active_ = true;
        // polling period
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
        // msg_format
        try{
            msg_format_ = MessageFormat::from_string(config.at("msg_format").get<std::string>());
        }catch(json::exception){
            msg_format_ = MessageFormat::Type::UNKN;
        }
    }

    void connect(){
        is_active_ = true;
        do_connect();
    }

    void disconnect(){
        do_disconnect();
    }

    void stop(){
        is_active_ = false;
        do_stop();
    }

    std::shared_ptr<Message> receive(){
        using namespace std::chrono;
        time_t now = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        if(stat_.last_in + polling_period_ > now){
            std::this_thread::sleep_for(seconds(stat_.last_in + polling_period_ - now));
        }
        Message msg = do_receive();
        ++stat_.count_in;
        stat_.last_in = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        return std::make_shared<Message>(std::move(msg));
    }

    void send(MessageWrapper & msg_w){
        using namespace std::chrono;
        do_send(msg_w);
        ++stat_.count_out;
        auto now = system_clock::now().time_since_epoch();
        stat_.last_out = duration_cast<seconds>(now).count();
    }

    ConnectorStat get_statistics(){
        return stat_;
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
/*
    static bool validate_config(json const & config){
        json const & schema = get_schema();
        for(auto const & iprop : schema.items()){
            if(iprop.value().value("required", false) && ! config.contains(iprop.key())){
                return false;
            }
        }
        return true;
    }
*/
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


#endif  // __M2E_BRIDGE_CONNECTOR_H__