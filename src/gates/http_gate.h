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
    ENABLED,
    DISABLED
};


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
    InternalQueue * queue_ {nullptr};
    ChannelState state_;
public:
    HTTPChannel() = default;
    HTTPChannel(string_view id, json const & config);
    void consume(char const * data, size_t n)const;
    bool verify_token(char const * token)const;
    bool is_anonymous()const { return token_.empty(); }
    bool is_malformed()const { return state_ == ChannelState::MALFORMED; };
    string const & get_id()const {return id_;}
    string const get_state_str()const
    {
        switch(state_){
            case ChannelState::MALFORMED: return "MALFORMED";
            case ChannelState::ENABLED: return "ENABLED";
            case ChannelState::DISABLED: return "DISABLED";
            case ChannelState::UNKN:
            default: return "UNKN";
        }
    }
    string const & get_queue_name()const {return queue_name_;}
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
    HTTPGate();  // private constructor to make class singleton
    HTTPChannelIterator channel_iterator_;
public:
    static void start();
    void stop() {};
    static bool create_channel(string const & id, string_view config);
    static bool update_channel(string const & id, string_view config);
    static bool delete_channel(string const & id);

    static HTTPChannel const & get_channel(string const & id)
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