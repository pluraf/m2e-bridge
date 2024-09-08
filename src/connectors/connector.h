#ifndef __M2E_BRIDGE_CONNECTOR_H__
#define __M2E_BRIDGE_CONNECTOR_H__

#include "nlohmann/json.hpp"
#include<iostream>

#include "m2e_message/message_wrapper.h"
#include "route.h"


enum class ConnectorMode {
    IN,
    OUT
};


class Connector {

protected:
    ConnectorMode mode_;
public:
    Connector(nlohmann::json json_descr, ConnectorMode mode) {
        mode_ = mode;
    }
    virtual void connect() {}
    virtual void disconnect() {}
    virtual void stop() {}
    virtual MessageWrapper* receive() { return nullptr; }
    virtual void send(const MessageWrapper &msg_w) {}
};


#endif