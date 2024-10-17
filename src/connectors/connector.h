#ifndef __M2E_BRIDGE_CONNECTOR_H__
#define __M2E_BRIDGE_CONNECTOR_H__

#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

#include "m2e_message/message_wrapper.h"
#include "route.h"


enum class ConnectorMode {
    IN,
    OUT
};


class Connector {

protected:
    ConnectorMode mode_;
    std::string pipeid_;

public:
    Connector(json const & json_descr, ConnectorMode mode, std::string pipeid) {
        mode_ = mode;
        pipeid_ = pipeid;
    }
    virtual void connect() {}
    virtual void disconnect() {}
    virtual void stop() {}
    virtual MessageWrapper * receive() { return nullptr; }
    virtual void send(MessageWrapper & msg_w) {}
};


#endif