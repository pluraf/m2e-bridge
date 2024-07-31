#ifndef __M2E_BRIDGE_CONDITION_H__
#define __M2E_BRIDGE_CONDITION_H__


#include "m2e_message/message_wrapper.h"


class Condition {
public:
    bool isTrue(const MessageWrapper &msg_w) { return true; }
};

#endif