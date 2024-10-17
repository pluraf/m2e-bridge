#ifndef __M2E_BRIDGE_TRANSFORMER_H__
#define __M2E_BRIDGE_TRANSFORMER_H__


#include "m2e_aliases.h"
#include "m2e_message/message_wrapper.h"


class Transformer {
public:
    Transformer(json const & json_descr) {}

    void pass(MessageWrapper &msg_w) {}
};


#endif  // __M2E_BRIDGE_TRANSFORMER_H__