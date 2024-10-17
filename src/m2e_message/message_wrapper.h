#ifndef __M2E_BRIDGE_MESSAGE_WRAPPER_H__
#define __M2E_BRIDGE_MESSAGE_WRAPPER_H__


#include <regex>

#include "m2e_aliases.h"
#include "message.h"


class MessageWrapper{
public:
    MessageWrapper(Message msg):orig_(msg),alt_(msg){}
    Message const & orig(){return orig_;}
    Message & alt(){return alt_;}

    string const & get_text()const{
        return alt_ ? alt_.get_text() : orig_.get_text();
    }
private:
    Message orig_;
    Message alt_;
};


#endif  // __M2E_BRIDGE_MESSAGE_WRAPPER_H__