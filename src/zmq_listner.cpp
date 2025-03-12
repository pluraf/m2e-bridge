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
#include "zmq_api.h"


//Initialize static member to null
ZmqListner * ZmqListner::instance_ = nullptr;


void ZmqListner::run(){
    try{
        context_ = zmq::context_t(2);
        socket_ = zmq::socket_t(context_, zmq::socket_type::rep);
        socket_.bind("ipc:///tmp/m2eb-zmq.sock");

        ZMQAPI api_handler {};
        while(! stop_){
            zmq::message_t request;
            auto res = socket_.recv(request);  // Wait for next request from client
            if(res){
                std::string reply_str = api_handler.handle_message(request);
                socket_.send(zmq::buffer(reply_str), zmq::send_flags::none);
            }
        }
        std::cout << "Zeromq shutting down..." << std::endl;
        socket_.close();
        context_.close();
    }catch(const zmq::error_t &e){
        if(e.num() == EINTR){
            std::cout << "Zeromq interrupted" << std::endl;
        }else{
            std::cerr << "Zeromq error: " << e.what() << std::endl;
        }
    }catch(const std::exception &e){
        std::cerr << "Zeromq exception: " << e.what() << std::endl;
    }
    std::cout << "Zeromq shut down..." << std::endl;
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
