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


extern "C"{
    #include <cbor.h>
}


#include "API_VERSION.h"
#include "zmq_api.h"
#include "api_helpers.h"
#include "gates/http_gate.h"
#include "global_config.h"
#include "m2e_exceptions.h"


string ZMQAPI::handle_message(zmq::message_t const & message)
{
    return api_handler((char const *)message.data(), message.size());
}


string ZMQAPI::execute_request(string_view method, string_view path, char const * payload, size_t payload_len){
    if(method == "GET" || method == "get"){
        if(path.starts_with("channel")){
            return zmq_channel_get(path, payload, payload_len);
        }else{
            return zmq_legacy_get(path);
        }
    }else if(method == "PUT" || method == "put"){
        if(path.starts_with("channel")){
            return zmq_channel_put(path, payload, payload_len);
        }else{
            return zmq_legacy_put(path);
        }
    }else if(method == "POST" || method == "post"){
        return zmq_channel_post(path, payload, payload_len);
    }else if(method == "DELETE" || method == "delete"){
        return zmq_channel_delete(path, payload, payload_len);
    }else{
        return "Unknown method";
    }
}


string ZMQAPI::api_handler(char const * buffer, size_t len)
{
    struct cbor_load_result result;
    cbor_item_t * request = cbor_load((unsigned char const *)buffer, len, &result);

    if(request == NULL) return "{\"error\": \"error\"}";
    if(! cbor_isa_array(request)){
        cbor_decref(&request);
        return "{\"error\": \"error\"}";
    }

    size_t array_size = cbor_array_size(request);
    if(array_size < 2){
        cbor_decref(& request);
        return "{\"error\": \"error\"}";
    }
    cbor_item_t ** items = cbor_array_handle(request);
    // Retrieve method
    cbor_item_t * item = items[0];
    if(! cbor_isa_string(item)){
        cbor_decref(&request);
        return "{\"error\": \"error\"}";
    }
    string_view method {(char *)cbor_string_handle(item), cbor_string_length(item)};
    // Retrieve path
    item = items[1];
    if(! cbor_isa_string(item)){
        cbor_decref(& request);
        return "{\"error\": \"error\"}";
    }
    string_view path {(char *)cbor_string_handle(item), cbor_string_length(item)};
    // Retrieve payload
    unsigned char * payload = NULL;
    size_t payload_len = 0;
    if(array_size > 2){
        item = items[2];
        if(cbor_isa_bytestring(item)){
            int d = cbor_bytestring_is_definite(item);
            payload = cbor_bytestring_handle(item);
            payload_len = cbor_bytestring_length(item);
        }else if(cbor_isa_string(item)){
            payload = cbor_string_handle(item);
            payload_len = cbor_string_length(item);
        }else{
            cbor_decref(& request);
            return "{\"error\": \"error\"}";
        }
    }

    string response = execute_request(method, path, (char *)payload, payload_len);
    cbor_decref(& request);
    return response;
}


string ZMQAPI::zmq_legacy_get(string_view path)
{
    if(path == "api_version"){
        return M2E_BRIDGE_VERSION;
    }else if(path == "status"){
        return "running";
    }
    return "unknown path";
}


string ZMQAPI::zmq_legacy_put(string_view path)
{
    if(path == "set_api_auth_off"){
        return gc.set_api_authentication(false) ? "ok" : "fail";
    }else if(path == "set_api_auth_on"){
        return gc.set_api_authentication(true) ? "ok" : "fail";
    }
    return "unknown path";
}


string ZMQAPI::zmq_channel_get(string_view path, char const * payload, size_t payload_len)
{
    vector<string> segments;
    segments = get_last_segments(path);
    if(segments.size() == 1){
        json j_channels = json::array();
        for(auto const & channel : HTTPGate::get_channels()){
            json j_channel = json::object();
            j_channel["id"] = channel.get_id();
            j_channel["type"] = "http";
            j_channel["state"] = channel.get_state_str();
            j_channel["enabled"] = channel.is_enabled();
            auto stat = channel.get_stat();
            j_channel["msg_received"] = stat.msg_received;
            j_channel["msg_timestamp"] = stat.msg_timestamp;
            j_channels.push_back(std::move(j_channel));
        }
        return j_channels.dump();
    }else if(segments.size() == 2){
        HTTPChannel const * channel {nullptr};
        try{
            channel = & HTTPGate::get_channel(segments[1]);
        }catch(std::out_of_range){
            return "{\"error\": \"error\"}";
        }
        json j_channel = json::object();
        j_channel["id"] = channel->get_id();
        j_channel["type"] = "http";
        if(channel->get_authtype() == AuthType::TOKEN){
            j_channel["token"] = channel->get_token();
        }
        j_channel["queue_name"] = channel->get_queue_name();
        j_channel["state"] = channel->get_state_str();
        j_channel["enabled"] = channel->is_enabled();
        j_channel["path"] = "/channel/http/" + channel->get_id();
        j_channel["authtype"] = channel->get_authtype_str();
        auto stat = channel->get_stat();
        j_channel["msg_received"] = stat.msg_received;
        j_channel["msg_timestamp"] = stat.msg_timestamp;
        return j_channel.dump();
    }else{
        return "{\"error\": \"error\"}";
    }
}


string ZMQAPI::zmq_channel_put(string_view path, char const * payload, size_t payload_len)
{
    auto segments = get_last_segments(path);
    if(segments.size() == 2){
        auto & channel_id = segments.back();
        try{
            if(HTTPGate::update_channel(channel_id, string_view(payload, payload_len))){
                return "";
            }
        }catch(configuration_error const & e){
            return fmt::format("{{\"error\": {}}}", e.what());
        }
    }
    return "{\"error\": \"error\"}";
}


string ZMQAPI::zmq_channel_post(string_view path, char const * payload, size_t payload_len)
{
    auto segments = get_last_segments(path);
    if(segments.size() == 2){
        auto & channel_id = segments.back();
        try{
            if(HTTPGate::create_channel(channel_id, string_view(payload, payload_len))) return "";
        }catch(configuration_error const & e){
            return fmt::format("{{\"error\": {}}}", e.what());
        }
    }
    return "{\"error\": \"error\"}";
}


string ZMQAPI::zmq_channel_delete(string_view path, char const * payload, size_t payload_len)
{
    auto segments = get_last_segments(path);
    if(segments.size() == 2){
        auto & channel_id = segments.back();
        if(HTTPGate::delete_channel(channel_id)){
            return "";
        }
    }
    return "{\"error\": \"error\"}";
}