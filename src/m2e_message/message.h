/* SPDX-License-Identifier: BSD-3-Clause */

/*
Copyright (c) 2024-2025 Pluraf Embedded AB <code@pluraf.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __M2E_BRIDGE_MESSAGE_H__
#define __M2E_BRIDGE_MESSAGE_H__


#include <cstddef>
#include <iostream>
#include <regex>

#include <cbor.h>

#include "m2e_aliases.h"


struct MessageFormat
{
    enum class Type {UNKN, RAW, JSON, CBOR};

    static std::string to_string(MessageFormat::Type v)
    {
        switch(v){
            case Type::UNKN: return "unkn";
            case Type::RAW: return "raw";
            case Type::JSON: return "json";
            case Type::CBOR: return "cbor";
            default: throw std::invalid_argument("Invalid MessageFormat");
        }
    }

    static MessageFormat::Type from_string(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });

        if(str == "unkn"){ return MessageFormat::Type::UNKN; }
        if(str == "raw"){ return MessageFormat::Type::RAW; }
        if(str == "json"){ return MessageFormat::Type::JSON; }
        if(str == "cbor"){ return MessageFormat::Type::CBOR; }

        throw std::invalid_argument("Unknown format: " + str);
    }
};


using bytes = std::vector<std::byte>;


class Message{
    bool is_valid_ {false};
    bytes payload_;
    std::string msg_raw_;
    std::string msg_topic_;
    mutable json decoded_json_;
    cbor_item_t *decoded_cbor_ {nullptr};
    mutable bool is_serialized_ {false};
    mutable std::vector<std::string> topic_levels_;
    string empty_level_ {""};
    MessageFormat::Type format_ {MessageFormat::Type::UNKN};
    map<string, string> attributes_;
public:
    Message() = default;

    Message(string const & data, MessageFormat::Type format, string const & topic = ""){
        is_serialized_ = true;
        msg_raw_ = data;
        payload_.assign(
            reinterpret_cast<std::byte const*>(data.data()),
            reinterpret_cast<std::byte const*>(data.data() + data.size())
        );
        msg_topic_ = topic;
        format_ = format;
        is_valid_ = true;
    }

    Message(vector<std::byte> const & data, MessageFormat::Type format, string const & topic = ""){
        is_serialized_ = true;
        msg_raw_.resize(data.size());
        for(int ix = 0; ix < data.size(); ++ix){
            msg_raw_[ix] = static_cast<char>(data[ix]);
        }
        msg_topic_ = topic;
        format_ = format;
        is_valid_ = true;
    }

    Message(json const & j_data, MessageFormat::Type format, string const & topic = ""){
        is_serialized_ = false;
        decoded_json_ = j_data;
        msg_topic_ = topic;
        format_ = format;
        is_valid_ = true;
    }

    Message(char const * data, size_t n, MessageFormat::Type format = MessageFormat::Type::RAW){
        is_serialized_ = true;
        msg_raw_ = string(data, data + n);
        format_ = format;
        is_valid_ = true;
    }

    Message(Message const & other){
        msg_raw_ = other.msg_raw_;
        payload_ = other.payload_;
        topic_levels_ = other.topic_levels_;
        is_serialized_ = other.is_serialized_;
        decoded_json_ = other.decoded_json_;
        attributes_ = other.attributes_;
        msg_topic_ = other.msg_topic_;
        format_ = other.format_;
        is_valid_ = other.is_valid_;
    }

    Message & operator=(Message const & other){
        if(this != & other){
            msg_raw_ = other.msg_raw_;
            payload_ = other.payload_;
            topic_levels_ = other.topic_levels_;
            is_serialized_ = other.is_serialized_;
            decoded_json_ = other.decoded_json_;
            attributes_ = other.attributes_;
            msg_topic_ = other.msg_topic_;
            format_ = other.format_;
            is_valid_ = other.is_valid_;
        }
        return * this;
    }

    Message(Message && other)noexcept:
        msg_raw_(std::move(other.msg_raw_)),
        payload_(std::move(other.payload_)),
        topic_levels_(std::move(other.topic_levels_)),
        is_serialized_(other.is_serialized_),
        decoded_json_(std::move(other.decoded_json_)),
        msg_topic_(std::move(other.msg_topic_)),
        attributes_(std::move(other.attributes_)),
        format_(other.format_),
        is_valid_(other.is_valid_) {}

    Message & operator=(Message && other)noexcept{
        if(this != & other){
            msg_raw_ = std::move(other.msg_raw_);
            payload_ = std::move(other.payload_);
            topic_levels_ = std::move(other.topic_levels_);
            is_serialized_ = other.is_serialized_;
            decoded_json_ = std::move(other.decoded_json_);
            msg_topic_ = std::move(other.msg_topic_);
            attributes_ = std::move(other.attributes_);
            format_ = other.format_;
            is_valid_ = other.is_valid_;
        }
        return * this;
    }

    ~Message()
    {
        if(decoded_cbor_){
            cbor_decref(&decoded_cbor_);
        }
    }

    MessageFormat::Type get_format()
    {
        return format_;
    }

    explicit operator bool()const{return is_valid_;}

    void set_attributes(map<string, string> const & attrs){
        attributes_ = attrs;
    }

    map<string, string> const & get_attributes(){
        return attributes_;
    }

    std::string & get_raw(){
        if(! is_serialized_){
            if(format_ == MessageFormat::Type::JSON){
                msg_raw_ = decoded_json_.dump();
            }else if(format_ == MessageFormat::Type::CBOR){
                vector<uint8_t> v = json::to_cbor(decoded_json_);
                msg_raw_ = string(v.begin(), v.end());
            }else{
                throw std::runtime_error("Unknown message format");
            }
            is_serialized_ = true;
        }

        return msg_raw_;
    }

    json & get_json(){
        if(is_serialized_){
           decoded_json_ = json::parse(msg_raw_);
           is_serialized_ = false;
           format_ = MessageFormat::Type::JSON;
        }
        return decoded_json_;
    }

    cbor_item_t *get_cbor()
    {
        if(is_serialized_){
            if(decoded_cbor_){ cbor_decref(&decoded_cbor_); }
            cbor_load_result res;
            decoded_cbor_ = cbor_load(
                reinterpret_cast<cbor_data>(payload_.data()),
                payload_.size(),
                &res
            );
            if(res.error.code != CBOR_ERR_NONE){
                throw std::runtime_error("Can not deserialize CBOR payload!");
            }
            is_serialized_ = false;
        }
        return decoded_cbor_;
    }

    std::string const & get_topic_level(int level){
        using namespace std;
        auto & levels = get_topic_levels();
        return level < levels.size() ? levels[level] : empty_level_;
    }

    std::vector<std::string> const & get_topic_levels(){
        using namespace std;
        if(topic_levels_.size() == 0){
            regex r("/");
            sregex_token_iterator it(msg_topic_.begin(), msg_topic_.end(), r, -1);
            sregex_token_iterator end;
            topic_levels_ = vector<std::string>(it, end);
        }
        return topic_levels_;
    }

    std::string const & get_topic()const{
        return msg_topic_;
    }
};


#endif  // __M2E_BRIDGE_MESSAGE_H__