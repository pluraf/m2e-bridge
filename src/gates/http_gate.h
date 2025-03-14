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


#ifndef __M2E_BRIDGE_HTTP_CHANNEL_H__
#define __M2E_BRIDGE_HTTP_CHANNEL_H__


#include <memory>

#include "CivetServer.h"
#include "m2e_aliases.h"
#include "internal_queue.h"


enum class ChannelState{
    UNKN,
    MALFORMED,
    CONFIGURED
};


enum class AuthType{
    UNKN,
    NONE,
    TOKEN
};


struct ChannelStat{
    size_t msg_received;
    size_t msg_timestamp;
};


class HTTPGate;


template <typename Key, typename Value>
class ConstIterator {
private:
    using MapIterator = typename std::unordered_map<Key, Value>::const_iterator;
    MapIterator it;
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Value;
    using difference_type = std::ptrdiff_t;
    using pointer = const Value *;
    using reference = const Value &;

    explicit ConstIterator(MapIterator i) : it(i) {}

    reference operator*() const { return it->second; }
    pointer operator->() const { return &(it->second); }

    ConstIterator& operator++() { ++it; return *this; }
    ConstIterator operator++(int) { ConstIterator temp = *this; ++it; return temp; }

    bool operator!=(const ConstIterator& other) const { return it != other.it; }
};


class HTTPChannel
{
    string id_;
    string token_;
    string queue_name_;
    AuthType authtype_ {AuthType::UNKN};
    InternalQueue * queue_ {nullptr};
    ChannelState state_ {ChannelState::UNKN};
    bool enabled_ {false};
    size_t msg_received_ {0};
    size_t msg_timestamp_ {0};

    AuthType str2authtype(string const & str){
        if(str == "token"){
            return AuthType::TOKEN;
        }else if(str == "none"){
            return AuthType::NONE;
        }
        return AuthType::UNKN;
    }
    void reconfigure(json const & config);
public:
    HTTPChannel() = default;
    HTTPChannel(string_view id, json const & config, bool strict=true);
    void consume(char const * data, size_t n);
    bool verify_token(char const * token)const;
    bool is_anonymous()const {return authtype_ == AuthType::NONE;}
    bool is_malformed()const {return state_ == ChannelState::MALFORMED;};
    bool is_enabled()const {return enabled_;}
    string const & get_id()const {return id_;}
    string const get_state_str()const
    {
        switch(state_){
            case ChannelState::MALFORMED: return "MALFORMED";
            case ChannelState::CONFIGURED: return "CONFIGURED";
            case ChannelState::UNKN:
            default: return "UNKN";
        }
    }
    string const & get_queue_name()const {return queue_name_;}
    string const & get_token()const {return token_;}
    AuthType get_authtype()const {return authtype_;}
    ChannelStat get_stat()const{
        return {.msg_received=msg_received_, .msg_timestamp=msg_timestamp_};
    }
    string get_authtype_str()const {
        switch (authtype_){
            case AuthType::NONE: return "none";
            case AuthType::TOKEN: return "token";
            case AuthType::UNKN:
            default: return "unkn";
        }
    }

    friend class HTTPGate;
};


class HTTPChannelIterator
{
    unordered_map<string, HTTPChannel> const & map_container_;
public:
    HTTPChannelIterator(unordered_map<string, HTTPChannel> const & map_container): map_container_(map_container) {}
    using Iterator = ConstIterator<string, HTTPChannel>;
    Iterator begin(){return Iterator(map_container_.cbegin());}
    Iterator end(){return Iterator(map_container_.cend());}
};


class HTTPGate
{
    static HTTPGate * instance_;
    std::unique_ptr<CivetServer> server_;
    unordered_map<string, HTTPChannel> channels_;
    HTTPChannelIterator channel_iterator_;
    bool enabled_ {false};

    HTTPGate();  // private constructor to make class singleton
    void save();
public:
    static void start();
    static void stop() {};
    static bool create_channel(string const & id, string_view config);
    static bool update_channel(string const & id, string_view config);
    static bool delete_channel(string const & id);

    static HTTPChannel & get_channel(string const & id)
    {
        return get_instance().channels_.at(id);
    }

    static HTTPChannelIterator & get_channels()
    {
        return get_instance().channel_iterator_;
    }

    static HTTPGate & get_instance()
    {
        if (instance_ == nullptr){
            instance_ = new HTTPGate();
        }
        return *instance_;
    }


};


#endif  // __M2E_BRIDGE_HTTP_CHANNEL_H__