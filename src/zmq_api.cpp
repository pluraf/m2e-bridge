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

#include "zmq_api.h"
#include "api_helpers.h"
#include "gates/http_gate.h"


string ZMQAPI::handle_message(zmq::message_t const & message)
{
    return api_handler((unsigned char const *)message.data(), message.size());
}


string ZMQAPI::execute_request(char const * method, char const * path, unsigned char const * payload, size_t payload_len){
    std::cout << method << " - " << path << std::endl;
    if(strcmp(method, "GET") == 0 || strcmp(method, "get") == 0){
        return zmq_channel_get(path, payload, payload_len);
    }else if(strcmp(method, "PUT") == 0 || strcmp(method, "put") == 0){
        return zmq_channel_put(path, payload, payload_len);
    }else if(strcmp(method, "POST") == 0 || strcmp(method, "post") == 0){
        return zmq_channel_post(path, payload, payload_len);
    }else if(strcmp(method, "DELETE") == 0 || strcmp(method, "delete") == 0){
        return zmq_channel_delete(path, payload, payload_len);
    }else{
        return "Unknown method";
    }
}


string ZMQAPI::api_handler(unsigned char const * buffer, size_t len)
{
    struct cbor_load_result result;
    cbor_item_t * request = cbor_load(buffer, len, &result);

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
    char method[20] = {0};
    if(cbor_string_length(item) >= 20){
        cbor_decref(& request);
        return "{\"error\": \"error\"}";
    }
    strncpy(method, (char *)cbor_string_handle(item), cbor_string_length(item));
    // Retrieve path
    item = items[1];
    if(! cbor_isa_string(item)){
        cbor_decref(& request);
        return "{\"error\": \"error\"}";
    }
    char path[200] = {0};
    if(cbor_string_length(item) >= 200){
        cbor_decref(& request);
        return "{\"error\": \"error\"}";
    }
    strncpy(path, (char *)cbor_string_handle(item), cbor_string_length(item));
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

    string response = execute_request(method, path, payload, payload_len);
    cbor_decref(& request);
    return response;
}


string ZMQAPI::zmq_channel_get(char const * path, unsigned char const * payload, size_t payload_len)
{
    vector<string> segments;
    segments = get_last_segments(path);
    if(segments.size() == 1){
        json j_channels = json::array();
        for(auto const & channel : HTTPGate::get_channels()){
            json j_channel = json::object();
            j_channel["id"] = channel.get_id();
            j_channel["state"] = channel.get_state_str();
            j_channel["msg_received"] = 0;
            j_channel["msg_timestamp"] = 0;
            j_channels.push_back(std::move(j_channel));
        }
        return j_channels.dump();
    }else if(segments.size() == 2){
        auto & channel = HTTPGate::get_channel(segments[1]);
        json j_channel = json::object();
        j_channel["id"] = channel.get_id();
        j_channel["queue_name"] = channel.get_queue_name();
        j_channel["state"] = channel.get_state_str();
        j_channel["msg_received"] = 0;
        j_channel["msg_timestamp"] = 0;
        return j_channel.dump();
    }else{
        return "{\"error\": \"error\"}";
    }
}


string ZMQAPI::zmq_channel_put(char const * path, unsigned char const * payload, size_t payload_len)
{
}


string ZMQAPI::zmq_channel_post(char const * path, unsigned char const * payload, size_t payload_len)
{
}


string ZMQAPI::zmq_channel_delete(char const * path, unsigned char const * payload, size_t payload_len)
{
}