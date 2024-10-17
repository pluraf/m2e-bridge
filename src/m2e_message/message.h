#ifndef __M2E_BRIDGE_MESSAGE_H__
#define __M2E_BRIDGE_MESSAGE_H__


#include <iostream>

#include "m2e_aliases.h"

class Message{
public:
    Message(const std::string &text, const std::string &topic):
        msg_text_(text),
        msg_topic_(topic){
    }

    operator bool()const{
        return msg_text_.size() > 0;
    }

    json & get_json()const{
        if(! is_json_payload_decoded_){
            try{
                decoded_json_payload_ = json::parse(msg_text_);
                is_json_payload_decoded_ = true;
            }catch(json::exception const & e){
                decoded_json_payload_ = json();
            }
        }
        return decoded_json_payload_;
    }

    std::string const & get_topic_level(int level)const{
        using namespace std;
        if(topic_levels_.size() == 0){
            regex r("/");
            sregex_token_iterator it(msg_topic_.begin(), msg_topic_.end(), r, -1);
            sregex_token_iterator end;
            topic_levels_ = vector<std::string>(it, end);
        }
        return level < topic_levels_.size() ? topic_levels_[level] : empty_level_;
    }

    std::string const & get_text()const{
        return msg_text_;
    }

    std::string const & get_topic()const{
        return msg_topic_;
    }
private:
    std::string msg_text_;
    std::string msg_topic_;
    mutable json decoded_json_payload_;
    mutable bool is_json_payload_decoded_ {false};
    mutable std::vector<std::string> topic_levels_;
    string empty_level_ {""};
};


#endif  // __M2E_BRIDGE_MESSAGE_H__