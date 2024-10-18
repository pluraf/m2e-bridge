#ifndef __M2E_BRIDGE_ZMQLISTENER_H__
#define __M2E_BRIDGE_ZMQLISTENER_H__


#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <fstream>
#include "VERSION.h"

enum class ZmqRequest{
    VERSION,
    NONE
};

inline ZmqRequest zmq_request_from_string(std::string request){
    if(request == "VERSION"){
        return ZmqRequest::VERSION;
    }else{
        return ZmqRequest::NONE;
    }
}

class ZmqListner{
private:
    std::thread *listner_th_ {nullptr};
    std::atomic<bool> stop_ {false};
    static ZmqListner* instance_;
    ZmqListner() {} // private constructor to make class singleton
    void run();
    std::string get_response(ZmqRequest req);
public:
    void start();
    void stop();
    static ZmqListner* get_instance(){
        if (instance_ == nullptr){
            instance_ = new ZmqListner();
        }
        return instance_;
    }
};
#endif