#ifndef __M2E_BRIDGE_CONNECTOR_H__
#define __M2E_BRIDGE_CONNECTOR_H__


#include "m2e_message/message_wrapper.h"
#include "route.h"


class Connector {
public:
    Connector(nlohmann::json json_descr) {
        using json = nlohmann::json;
    }
    void connect() {}
    void disconnect() {}
    MessageWrapper receive() {}
    void send(const MessageWrapper &msg_w) {}
};


#endif