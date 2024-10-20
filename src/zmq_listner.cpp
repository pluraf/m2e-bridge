#include "zmq_listner.h"


//Initialize static member to null
ZmqListner* ZmqListner::instance_ = nullptr;


void ZmqListner::run(){
    try{
        context_ = zmq::context_t(2);
        socket_ = zmq::socket_t(context_, zmq::socket_type::rep);
        socket_.bind("ipc:///tmp/m2e-bridge-zmq.sock");

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
        case ZmqRequest::VERSION :{
            return M2E_BRIDGE_VERSION;
            break;
        }
        default:
            return "Hello from m2e-bridge";
    }
}
