/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


#ifndef __M2E_BRIDGE_MESSAGE_H__
#define __M2E_BRIDGE_MESSAGE_H__


#include <cstddef>
#include <iostream>
#include <regex>

#include "m2e_aliases.h"


enum class MessageFormat {UNKN, RAW, JSON, CBOR};


class Message{
    bool is_valid_ {false};
    std::string msg_raw_;
    std::string msg_topic_;
    mutable json decoded_json_;
    mutable bool is_serialized_ {false};
    mutable std::vector<std::string> topic_levels_;
    string empty_level_ {""};
    MessageFormat format_ {MessageFormat::UNKN};
public:
    Message() = default;

    Message(string const & data, string const & topic = ""){
        is_serialized_ = true;
        msg_raw_ = data;
        msg_topic_ = topic;
        is_valid_ = true;
    }

    Message(vector<std::byte> const & data, string const & topic = ""){
        is_serialized_ = true;
        msg_raw_.resize(data.size());
        for(int ix = 0; ix < data.size(); ++ix){
            msg_raw_[ix] = static_cast<char>(data[ix]);
        }
        msg_topic_ = topic;
        is_valid_ = true;
    }

    Message(json const & j_data, MessageFormat format, string const & topic = ""){
        is_serialized_ = false;
        decoded_json_ = j_data;
        msg_topic_ = topic;
        format_ = format;
        is_valid_ = true;
    }

    Message(char const * data, size_t n){
        is_serialized_ = true;
        msg_raw_ = string(data, data + n);
        is_valid_ = true;
    }

    Message(Message const & other){
        msg_raw_ = other.msg_raw_;
        topic_levels_ = other.topic_levels_;
        is_serialized_ = other.is_serialized_;
        decoded_json_ = other.decoded_json_;
        msg_topic_ = other.msg_topic_;
        format_ = other.format_;
        is_valid_ = other.is_valid_;
    }

    Message & operator=(Message const & other){
        if(this != & other){
            msg_raw_ = other.msg_raw_;
            topic_levels_ = other.topic_levels_;
            is_serialized_ = other.is_serialized_;
            decoded_json_ = other.decoded_json_;
            msg_topic_ = other.msg_topic_;
            format_ = other.format_;
            is_valid_ = other.is_valid_;
        }
        return * this;
    }

    Message(Message && other)noexcept:
        msg_raw_(std::move(other.msg_raw_)),
        topic_levels_(std::move(other.topic_levels_)),
        is_serialized_(other.is_serialized_),
        decoded_json_(std::move(other.decoded_json_)),
        msg_topic_(std::move(other.msg_topic_)),
        format_(other.format_),
        is_valid_(other.is_valid_) {}

    Message & operator=(Message && other)noexcept{
        if(this != & other){
            msg_raw_ = std::move(other.msg_raw_);
            topic_levels_ = std::move(other.topic_levels_);
            is_serialized_ = other.is_serialized_;
            decoded_json_ = std::move(other.decoded_json_);
            msg_topic_ = std::move(other.msg_topic_);
            format_ = other.format_;
            is_valid_ = other.is_valid_;
        }
        return * this;
    }

    explicit operator bool()const{return is_valid_;}

    std::string & get_raw(){
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
        if(is_serialized_){
           decoded_json_ = json::parse(msg_raw_);
           is_serialized_ = false;
           format_ = MessageFormat::JSON;
        }
        return decoded_json_;
    }

    std::string const & get_topic_level(int level){
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
};


#endif  // __M2E_BRIDGE_MESSAGE_H__