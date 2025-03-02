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


#include "zmq_listner.h"
#include "global_config.h"


//Initialize static member to null
ZmqListner* ZmqListner::instance_ = nullptr;


void ZmqListner::run(){
    try{
        context_ = zmq::context_t(2);
        socket_ = zmq::socket_t(context_, zmq::socket_type::rep);
        socket_.bind("ipc:///tmp/m2eb-zmq.sock");

        while(!stop_){
            zmq::message_t request;
            auto res = socket_.recv(request);  // Wait for next request from client
            if(res){
                std::string received_msg = request.to_string();
                std::cout << "Received Zeromq request: " << received_msg << std::endl;

                std::string reply_str = get_response(zmq_request_from_string(received_msg));
                //  Send reply back to client
                zmq::message_t reply (reply_str.size());
                memcpy(reply.data(), reply_str.c_str(), reply_str.size());
                socket_.send(reply, zmq::send_flags::none);
            }
        }
        std::cout << "Zeromq Shutting down..." << std::endl;
        socket_.close();
        context_.close();
    }catch(const zmq::error_t &e){
        if(e.num() == EINTR){
            std::cout << "Zeromq Interrupted" << std::endl;
        }else{
            std::cerr << "Zeromq Error: " << e.what() << std::endl;
        }
    }catch(const std::exception &e){
        std::cerr << "Zeromq Exception: " << e.what() << std::endl;
    }
    std::cout << "Zeromq Shutting down..." << std::endl;
}

void ZmqListner::start(){
    if(listner_th_ == nullptr){
        stop_ = false;
        listner_th_ = new std::thread(&ZmqListner::run, this);
    }
}


void ZmqListner::stop(){
    stop_ = true;
    context_.shutdown();
    socket_.close();
    context_.close();
    if(listner_th_ != nullptr){
        listner_th_->join();
        delete listner_th_;
        listner_th_ = nullptr;
    }
}


std::string ZmqListner::get_response(ZmqRequest req){
    switch(req){
        case ZmqRequest::API_VERSION:
            return M2E_BRIDGE_VERSION;
        case ZmqRequest::STATUS:
            return "running";
        case ZmqRequest::SET_API_AUTH_OFF:
            if(gc.set_api_authentication(false)){
                return "ok";
            }
            else{
                return "fail";
            }

        case ZmqRequest::SET_API_AUTH_ON:
            if(gc.set_api_authentication(true)){
                return "ok";
            }
            else{
                return "fail";
            }
        default:
            return "Hello from m2e-bridge";
    }
}
