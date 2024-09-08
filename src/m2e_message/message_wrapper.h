#ifndef __M2E_BRIDGE_MESSAGE_WRAPPER_H__
#define __M2E_BRIDGE_MESSAGE_WRAPPER_H__


#include "m2e_aliases.h"


#include "message.h"


class MessageWrapper {
public:
    MessageWrapper(const Message msg): msg(msg) { }
    json const & get_payload(){
        if(! is_payload_decoded_){
            decoded_payload_ = json::parse(msg.get_msg_text());
            is_payload_decoded_ = true;
        }
        return decoded_payload_;
    }
    Message const msg;
private:
    json decoded_payload_;
    bool is_payload_decoded_ {false};
};


#endif