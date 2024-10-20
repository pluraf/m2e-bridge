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
    NONE
};


inline ZmqRequest zmq_request_from_string(std::string request){
    if(request == "api_version"){
        return ZmqRequest::API_VERSION;
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