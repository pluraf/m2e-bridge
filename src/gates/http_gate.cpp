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


#include <stdexcept>
#include <iostream>
#include <tuple>
#include <utility>
#include <string_view>

#include "http_gate.h"
#include "civet_helpers.h"
#include "global_config.h"


HTTPGate * HTTPGate::instance_ = nullptr;


class AuthHandler: public CivetAuthHandler{
    HTTPGate & gate_;
public:
    AuthHandler(HTTPGate & gate): gate_(gate) {}

    bool authorize(CivetServer *server, struct mg_connection *conn)override{
        mg_request_info const * req_info = mg_get_request_info(conn);
        char const *  channel_id = cv_get_last_segment(req_info);
        bool authorized {false};
        if(channel_id){
            try{
                HTTPChannel const & channel = gate_.get_channel(channel_id);
                if(channel.is_anonymous()) return true;
                char const * token = cv_get_bearer_token(conn);
                authorized = token && channel.verify_token(token);
                if(! authorized) mg_send_http_error(conn, 401, "Not authorized");
            }catch(std::out_of_range){
                mg_send_http_error(conn, 404, "%s", "Channel not found!");
            }
        }else{
            mg_send_http_error(conn, 400, "Channel ID is missing in URI");
        }
        return authorized;
    }
private:
    std::string* public_key_;
    bool allow_anonymous_ {false};
};


class ApiHandler: public CivetHandler{
    HTTPGate & gate_;
public:
    ApiHandler(HTTPGate & gate): gate_(gate) {}

    bool handlePost(CivetServer * server, struct mg_connection * conn)override{
        json pipeline_data;
        const struct mg_request_info * req_info = mg_get_request_info(conn);
        char const *  channel_id = cv_get_last_segment(req_info);
        if(channel_id){
            try{
                HTTPChannel const & channel = gate_.get_channel(channel_id);
                char buf[1024];
                int length = mg_read(conn, buf, sizeof(buf));
                channel.consume(buf, length);
                mg_send_http_ok(conn, "text/plain", 0);
            }catch(std::out_of_range){
                mg_send_http_error(conn, 404, "%s", "Pipeline not found!");
            }
        }else{
            mg_send_http_error(conn, 400, "Channel ID is missing in URI");
        }
        return true;
    }

    bool handleGet(CivetServer * server, struct mg_connection * conn)override{
        mg_send_http_error(conn, 405, "%s", "Method Not Allowed");
        return true;
    }
};


HTTPChannel::HTTPChannel(string_view id, json const & config)
{
    id_ = id;
    if(config.contains("token")){
        token_ = config.at("token").get<string>();
    }else{
        token_ = config.value("secret", "");
    }
    try{
        queue_name_ = config.at("queue_name").get<string>();
        queue_ = InternalQueue::get_queue_ptr(queue_name_);
    }catch(json::exception){
        state_ = ChannelState::MALFORMED;
    }
}


void HTTPChannel::consume(char const * data, size_t n)const
{
    queue_->push(Message(data, n));
    std::cout<<"consumed " << n << " bytes" << std::endl;
}


bool HTTPChannel::verify_token(char const * token)const
{
    return token_ == token;
}


HTTPGate::HTTPGate():channel_iterator_(channels_)
{
    json const & config = gc.get_http_gate_config();
    if(config.contains("channels")){
        auto config_channels = config["channels"];
        for(auto it = config_channels.cbegin(); it != config_channels.cend(); ++it){
            HTTPChannel ch {it.key(), * it};
            if(! ch.is_malformed()) channels_[it.key()] = std::move(ch);
        }
    }
}


void HTTPGate::start()
{
    char const * options[] = {
        "listening_ports", "8010",
        "num_threads", "2",
        NULL
    };

    auto & instance = get_instance();
    instance.server_ = std::make_unique<CivetServer>(options);
    instance.server_->addAuthHandler("/channel/http/", new AuthHandler(*instance_));
    instance.server_->addHandler("/channel/http/", new ApiHandler(*instance_));
}


bool HTTPGate::create_channel(string const & id, string_view config)
{
    auto j_channel = json::parse(config);
    HTTPChannel ch {id, j_channel};
    if(! ch.is_malformed()){
        auto & instance = get_instance();
        instance.channels_[string(id)] = std::move(ch);
        return true;
    }
    return false;
}


bool HTTPGate::update_channel(string const & id, string_view config)
{
    auto j_channel = json::parse(config);
    HTTPChannel ch {id, j_channel};
    if(! ch.is_malformed()){
        auto & instance = get_instance();
        instance.channels_.erase(id);
        instance.channels_[id] = std::move(ch);
        return true;
    }
    return false;
}


bool HTTPGate::delete_channel(string const & id)
{
    auto & instance = get_instance();
    instance.channels_.erase(id);
    return true;
}
