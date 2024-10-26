#ifndef __M2E_BRIDGE_MESSAGE_H__
#define __M2E_BRIDGE_MESSAGE_H__


#include <iostream>

#include "m2e_aliases.h"


enum class MessageFormat {UNKN, RAW, JSON};


class Message{
public:
    Message() = default;
    Message(string const &text, string const & topic):
        msg_text_(text),
        msg_topic_(topic){
    }

    std::string const & get_raw(){
        if(! is_serialized_){
            if(decoder_ == MessageFormat::JSON){
                msg_text_ = decoded_json_.dump();
                is_serialized_ = true;
            }
        }
        return msg_text_;
    }

    json & get_json(){
        if(decoder_ == MessageFormat::UNKN || decoder_ == MessageFormat::JSON){
            decoder_ = MessageFormat::JSON;
        }else{
            throw std::runtime_error("Decoder is already set to a different type");
        }

        is_serialized_ = false;

        if(! is_deserialized_){
            try{
                decoded_json_ = json::parse(msg_text_);
                is_deserialized_ = true;
            }catch(json::exception const & e){
                decoded_json_ = json();
            }
        }
        return decoded_json_;
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

    std::string const & get_topic()const{
        return msg_topic_;
    }
private:
    std::string msg_text_;
    std::string msg_topic_;
    mutable json decoded_json_;
    mutable bool is_serialized_ {false};
    mutable bool is_deserialized_ {false};
    mutable std::vector<std::string> topic_levels_;
    string empty_level_ {""};
    MessageFormat decoder_ {MessageFormat::UNKN};
    MessageFormat encoder_ {MessageFormat::UNKN};
};


#endif  // __M2E_BRIDGE_MESSAGE_H__