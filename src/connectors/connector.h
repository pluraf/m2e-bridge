#ifndef __M2E_BRIDGE_CONNECTOR_H__
#define __M2E_BRIDGE_CONNECTOR_H__

#include "nlohmann/json.hpp"
#include<iostream>

#include "m2e_message/message_wrapper.h"
#include "route.h"

class Connector {

protected:
    std::string connector_type_; 
public:
    Connector(nlohmann::json json_descr, std::string type) {
        connector_type_ = type;
    }
    virtual void connect() {}
    virtual void disconnect() {}
    virtual MessageWrapper* receive() {}
    virtual void send(const MessageWrapper &msg_w) {}
};


#endif