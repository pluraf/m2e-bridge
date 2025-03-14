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
#include "m2e_exceptions.h"


HTTPGate * HTTPGate::instance_ = nullptr;


class HTTPChannelAuthHandler: public CivetAuthHandler{
    HTTPGate & gate_;
public:
    HTTPChannelAuthHandler(HTTPGate & gate): gate_(gate) {}

    bool authorize(CivetServer *server, struct mg_connection *conn)override{
        mg_request_info const * req_info = mg_get_request_info(conn);
        char const *  channel_id = cv_get_last_segment(req_info);
        bool authorized {false};
        if(channel_id){
            try{
                HTTPChannel const & channel = gate_.get_channel(channel_id);
                if(channel.is_malformed()){
                    mg_send_http_error(conn, 500, "%s", "Channel is malformed!");
                    return authorized;
                }
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


class HTTPChannelApiHandler: public CivetHandler{
    HTTPGate & gate_;
public:
    HTTPChannelApiHandler(HTTPGate & gate): gate_(gate) {}

    bool handlePost(CivetServer * server, struct mg_connection * conn)override{
        json pipeline_data;
        const struct mg_request_info * req_info = mg_get_request_info(conn);
        char const *  channel_id = cv_get_last_segment(req_info);
        if(channel_id){
            try{
                HTTPChannel & channel = gate_.get_channel(channel_id);
                if(! channel.is_enabled()){
                    mg_send_http_error(conn, 405, "%s", "Channel is disabled!");
                    return true;
                }
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


HTTPChannel::HTTPChannel(string_view id, json const & config, bool strict)
{
    auto state = ChannelState::CONFIGURED;  // Let's be optimistic
    id_ = id;
    authtype_ = str2authtype(config.at("authtype").get<string>());
    if(authtype_ == AuthType::TOKEN){
        if(config.contains("token")){
            token_ = config.at("token").get<string>();
        }else{
            token_ = config.at("secret").get<string>();
        }
        if(token_.empty()){
            if(strict) throw configuration_error("token can not be empty");
            state = ChannelState::MALFORMED;
        }
    }
    enabled_ = config.value("enabled", false);
    try{
        queue_name_ = config.at("queue_name").get<string>();
        if(queue_name_.empty()){
            if(strict) throw configuration_error("queue_name can not be empty");
            state = ChannelState::MALFORMED;
        }else{
            queue_ = InternalQueue::get_queue_ptr(queue_name_);
        }
    }catch(json::exception){
        state = ChannelState::MALFORMED;
    }
    state_ = state;
}


void HTTPChannel::reconfigure(json const & config){
    HTTPChannel tmp = * this;
    if(config.contains("queue_name")){
        tmp.queue_name_ = config.at("queue_name").get<string>();
        if(tmp.queue_name_.empty()) throw configuration_error("queue_name can not be empty");
    }else if (queue_name_.empty()){
        throw configuration_error("queue_name can not be empty");
    }
    if(config.contains("authtype")){
        tmp.authtype_ = str2authtype(config.at("authtype").get<string>());
        if(tmp.authtype_ == AuthType::TOKEN){
            if(config.contains("token")){
                tmp.token_ = config.at("token").get<string>();
            }else if(config.contains("secret")){
                tmp.token_ = config.at("secret").get<string>();
            }else if(authtype_ != tmp.authtype_){
                throw configuration_error("Token is missing");
            }
            if(tmp.token_.empty()) throw configuration_error("token can not be empty");
        }
    }else if(authtype_ == AuthType::TOKEN && token_.empty()){
        throw configuration_error("token can not be empty");
    }
    if(config.contains("enabled")) tmp.enabled_ = config.at("enabled").get<bool>();
    // All good accept new parameters
    queue_name_ = tmp.queue_name_;
    queue_ = InternalQueue::get_queue_ptr(queue_name_);
    authtype_ = tmp.authtype_;
    token_ = tmp.token_;
    enabled_ = tmp.enabled_;
    state_ = ChannelState::CONFIGURED;
}


void HTTPChannel::consume(char const * data, size_t n)
{
    queue_->push(Message(data, n));
    ++msg_received_;
    auto now = chrono::system_clock::now().time_since_epoch();
    msg_timestamp_ = chrono::duration_cast<chrono::seconds>(now).count();
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
            try{
                HTTPChannel ch {it.key(), * it, false};
                channels_[it.key()] = std::move(ch);
            }catch(...){
                continue;
            }
        }
    }
    enabled_ = config.at("enabled").get<bool>();
}


void HTTPGate::save()
{
    json channels(json::value_t::object);
    for(auto it = channels_.cbegin(); it != channels_.cend(); ++it){
        auto & [chanid, channel] = *it;
        json ch_config(json::value_t::object);
        ch_config["queue_name"] = channel.queue_name_;
        if(! channel.is_anonymous()) ch_config["token"] = channel.token_;
        ch_config["enabled"] = channel.enabled_;
        ch_config["authtype"] = channel.get_authtype_str();
        channels[chanid] = ch_config;
    }

    ordered_json j_gate = {
        {"enabled", enabled_},
        {"channels", channels}
    };
    gc.save_http_gate(j_gate);
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
    instance.server_->addAuthHandler("/channel/http/", new HTTPChannelAuthHandler(*instance_));
    instance.server_->addHandler("/channel/http/", new HTTPChannelApiHandler(*instance_));
}


bool HTTPGate::create_channel(string const & id, string_view config)
{
    auto & instance = get_instance();
    if(instance_->channels_.contains(id)){
        throw configuration_error("Channel already exist");
    }
    auto j_channel = json::parse(config);
    HTTPChannel ch {id, j_channel};
    if(! ch.is_malformed()){
        instance.channels_[id] = std::move(ch);
        instance.save();
        return true;
    }
    return false;
}


bool HTTPGate::update_channel(string const & id, string_view config)
{
    auto j_channel = json::parse(config);
    get_channel(id).reconfigure(j_channel);
    get_instance().save();
    return true;
}


bool HTTPGate::delete_channel(string const & id)
{
    auto & instance = get_instance();
    instance.channels_.erase(id);
    instance.save();
    return true;
}
