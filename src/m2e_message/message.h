#ifndef __M2E_BRIDGE_MESSAGE_H__
#define __M2E_BRIDGE_MESSAGE_H__


#include <iostream>

#include "m2e_aliases.h"


enum class MessageFormat {UNKN, RAW, JSON, CBOR};


class Message{
public:
    Message() = default;

    Message(string const & data, string const & topic){
        is_serialized_ = true;
        is_deserialized_ = false;
        msg_raw_ = data;
        msg_topic_ = topic;
        is_valid_ = true;
    }

    Message(json const & j_data, string const & topic, MessageFormat format){
        is_serialized_ = false;
        is_deserialized_ = true;
        decoded_json_ = j_data;
        msg_topic_ = topic;
        format_ = format;
        is_valid_ = true;
    }

    std::string const & get_raw(){
        if(! is_serialized_){
            if(format_ == MessageFormat::JSON){
                msg_raw_ = decoded_json_.dump();
            }else if(format_ == MessageFormat::CBOR){
                vector<uint8_t> v = json::to_cbor(decoded_json_);
                msg_raw_ = string(v.begin(), v.end());
            }else{
                throw std::runtime_error("Unknown encoder");
            }
            is_serialized_ = true;
        }

        return msg_raw_;
    }

    json & get_json(){
        if(format_ == MessageFormat::UNKN){
            throw std::runtime_error("Decoder is already set to a different type");
        }

        if(! is_deserialized_){
            if(format_ == MessageFormat::JSON){
                decoded_json_ = json::parse(msg_raw_);
            }else if(format_ == MessageFormat::CBOR){
                decoded_json_ = json::from_cbor(msg_raw_);
            }else{
                throw std::logic_error("Message can not be represented as JSON");
            }
            is_deserialized_ = true;
            is_serialized_ = false;
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

    operator bool()const{
        return is_valid_;
    }

private:
    bool is_valid_ {false};
    std::string msg_raw_;
    std::string msg_topic_;
    mutable json decoded_json_;
    mutable bool is_serialized_ {false};
    mutable bool is_deserialized_ {false};
    mutable std::vector<std::string> topic_levels_;
    string empty_level_ {""};
    MessageFormat format_ {MessageFormat::UNKN};
};


#endif  // __M2E_BRIDGE_MESSAGE_H__