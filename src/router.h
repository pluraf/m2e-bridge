#ifndef __M2E_BRIDGE_ROUTER_H__
#define __M2E_BRIDGE_ROUTER_H__


#include "m2e_message/message_wrapper.h"
#include "route.h"


class Router {
public:
    Route getRoute(const MessageWrapper &msg_w) { return Route(); }
};


#endif