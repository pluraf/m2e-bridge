#ifndef __M2E_BRIDGE_MESSAGE_H__
#define __M2E_BRIDGE_MESSAGE_H__

#include <iostream>


class Message {
    std::string msg_text_;
    std::string msg_topic_;

public:
    Message(const std::string &text, const std::string &topic)
        :msg_text_(text),
         msg_topic_(topic){
    }

    std::string const & get_text()const{
        return msg_text_;
    }

    std::string const & get_topic()const{
        return msg_topic_;
    }

};


#endif