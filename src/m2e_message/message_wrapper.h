#ifndef __M2E_BRIDGE_MESSAGE_WRAPPER_H__
#define __M2E_BRIDGE_MESSAGE_WRAPPER_H__


#include "message.h"


class MessageWrapper {
public:
    MessageWrapper(const Message msg): msg(msg) { }

    Message msg;

};


#endif