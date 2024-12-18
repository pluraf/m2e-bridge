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


#ifndef __M2E_BRIDGE_ZMQ_LISTENER_H__
#define __M2E_BRIDGE_ZMQ_LISTENER_H__


#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <fstream>

#include "API_VERSION.h"


enum class ZmqRequest{
    API_VERSION,
    STATUS,
    SET_API_AUTH_ON,
    SET_API_AUTH_OFF,
    NONE
};


inline ZmqRequest zmq_request_from_string(std::string request){
    if(request == "api_version"){
        return ZmqRequest::API_VERSION;
    }else if(request == "status"){
        return ZmqRequest::STATUS;
    }else if(request == "set_api_auth_on"){
        return ZmqRequest::SET_API_AUTH_ON;
    }else if(request == "set_api_auth_off"){
        return ZmqRequest::SET_API_AUTH_OFF;
    }else{
        return ZmqRequest::NONE;
    }
}


class ZmqListner{
public:
    void start();
    void stop();
    static ZmqListner* get_instance(){
        if(instance_ == nullptr){
            instance_ = new ZmqListner();
        }
        return instance_;
    }

private:
    std::thread *listner_th_ {nullptr};
    std::atomic<bool> stop_ {false};
    static ZmqListner* instance_;
    ZmqListner(){} // private constructor to make class singleton
    void run();
    std::string get_response(ZmqRequest req);
    zmq::socket_t socket_;
    zmq::context_t context_;
};


#endif  // __M2E_BRIDGE_ZMQ_LISTENER_H__