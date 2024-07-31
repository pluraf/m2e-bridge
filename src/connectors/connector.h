#ifndef __M2E_BRIDGE_CONNECTOR_H__
#define __M2E_BRIDGE_CONNECTOR_H__


#include "m2e_message/message.h"
#include "route.h"


class Connector {
public:
    void start() {}
    void stop() {}
    Message receive() {}
    void send(const Message &msg, const Route &route) {}
};


#endif