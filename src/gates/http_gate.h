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
    MALFORMED
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
    HTTPChannel(string const & id, json const & config);
    void consume(char const * data, size_t n)const;
    bool verify_token(char const * token)const;
    bool is_anonymous()const { return token_.empty(); }
    bool is_malformed()const { return state_ == ChannelState::MALFORMED; };
};


class HTTPGate
{
    std::unique_ptr<CivetServer> server_;
    unordered_map<string, HTTPChannel> channels_;
public:
    HTTPGate();
    void start();
    void stop() {};
    HTTPChannel const & get_channel(string const & id) { return channels_.at(id); }
};


#endif  // __M2E_BRIDGE_HTTP_CHANNEL_H__