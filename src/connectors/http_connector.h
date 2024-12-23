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

#ifndef __M2E_BRIDGE_HTTP_CONNECTOR_H__
#define __M2E_BRIDGE_HTTP_CONNECTOR_H__

#include <string>
#include <curl/curl.h>
#include <string>
#include <regex>
#include <thread>
#include <chrono>

#include <curl/curl.h>

#include "connector.h"
#include "database.h"

inline std::string base64_encode(const std::string& input) {
    static const char encode_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(encode_table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) encoded.push_back(encode_table[((val << 8) >> (valb + 8)) & 0x3F]);
    while (encoded.size() % 4) encoded.push_back('=');
    return encoded;
}

enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE
};

inline HttpMethod string_to_enum(const std::string& method) {
    // Mapping of strings to enum values
    static const std::unordered_map<std::string, HttpMethod> method_map = {
        {"GET", HttpMethod::GET},
        {"POST", HttpMethod::POST},
        {"PUT", HttpMethod::PUT},
        {"DELETE", HttpMethod::DELETE}
    };

    auto it = method_map.find(method);
    if (it != method_map.end()) {
        return it->second; // Return the corresponding enum value
    }
    throw std::invalid_argument("Invalid HTTP method: " + method);
}

inline bool is_valid_http_url(const std::string& url) {
    // Regular expression for matching HTTP/HTTPS URLs
    const std::regex url_pattern(R"(^(http:\/\/|https:\/\/)[a-zA-Z0-9\-\.]+(\:[0-9]+)?(\/[^\s\?]*)?(\?[^\s]*)?$)");
    return std::regex_match(url, url_pattern);
}

class HttpConnector: public Connector {
private:
    std::string url_;
    bool verify_cert_;
    HttpMethod method_;
    json header_;
    std::string payload_;
    std::string authbundle_id_;
    int request_freq_limit_;

    CURL * curl;

    struct HttpResponse{
        CURLcode curl_code;
        std::string resp_str;
        std::string content_type;
    };



    void parse_authbundle(){

        Database db;
        AuthBundle ab;
        bool res = db.retrieve_authbundle(authbundle_id_, ab);
        if(res){
            if(ab.service_type != ServiceType::HTTP){
                throw std::runtime_error("Incompatiable authbundle service type");
            }
            if(ab.auth_type == AuthType::BASIC){
                std::string credentials = ab.username + ":" + ab.password;
                std::string encoded_credentials = base64_encode(credentials);
                header_["Authorization"]= "Basic "+ encoded_credentials;

            }else if(ab.auth_type == AuthType::BEARER){
                header_["Authorization"]= "Bearer "+ ab.password;
            }else{
                throw std::runtime_error("Incompatiable authbundle auth type");
            }
        }
        else{
            throw std::runtime_error("Not able to retreive bundle ");
        }
    }

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        std::string* response = static_cast<std::string*>(userp);
        response->append(static_cast<char*>(contents), totalSize);
        return totalSize; // Return the size of the data written
    }

    HttpResponse send_request(){
        HttpResponse response;
        curl_easy_setopt(curl, CURLOPT_URL, url_.c_str());
        switch(method_){
            case HttpMethod::POST:
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                break;
            case HttpMethod::PUT:
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
                break;
            case HttpMethod::DELETE:
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
                break;
            case HttpMethod::GET:
            default: break;
        }

        if(verify_cert_){
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        }
        else{
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        }

        struct curl_slist* headers = nullptr;
        for (auto it = header_.begin(); it != header_.end(); ++it) {
            if (!it.value().is_string()){
                throw std::runtime_error("Only string values allowed in attribute:header in http connector ");
            }
            headers = curl_slist_append(headers, (it.key() +": "+ std::string(it.value())).c_str());
        }
        if(payload_ != ""){
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_.c_str());
        }
        if (headers) {
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.resp_str);

        response.curl_code = curl_easy_perform(curl);
        if(response.curl_code == CURLE_OK){
            char* content_type = nullptr;
            curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
            response.content_type = std::string(content_type);
        }
        if (headers) {
            curl_slist_free_all(headers);
        }

        return response;
    }


public:
    HttpConnector(std::string pipeid, ConnectorMode mode_, json const & json_descr):
    Connector(pipeid, mode_, json_descr){
        try {
            url_ = json_descr.at("url").get<std::string>();
            if(!is_valid_http_url(url_)){
                throw std::runtime_error("Invalid url format");
            }
        }catch(json::exception) {
            throw std::runtime_error("url cannot be null for http connector");
        }

        try {
            verify_cert_ = json_descr.at("https_verify_cert").get<bool>();
        }catch(json::exception) {
            verify_cert_ = true;
        }

        try {
            std::string method_str = json_descr.at("method").get<std::string>();
            method_ = string_to_enum(method_str);
        }catch(json::exception) {
            throw std::runtime_error("method cannot be null for http connector");
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error(e.what());
        }

        if(json_descr.contains("header")) {
            if(json_descr["header"].is_object()){
                header_ = json_descr["header"];
                for (auto it = header_.begin(); it != header_.end(); ++it) {
                    if (!it.value().is_string()){
                        throw std::runtime_error("Only string values allowed in attribute:header in http connector ");
                    }
                }
            }else{
                throw std::runtime_error("Invalid format for attribute:header in http connector ");
            }
        }

        if(json_descr.contains("payload")) {
            if(method_ == HttpMethod::GET){
                throw std::runtime_error("Http connector GET method cannot have a payload");
            }
            if(json_descr["payload"].is_string()){
                header_["Content-Type"] = "text/plain";
                payload_ = json_descr["payload"];
                header_["Content-Length"]= std::to_string(payload_.size());
            }else if(json_descr["payload"].is_object() || json_descr["payload"].is_array()){
                header_["Content-Type"] = "application/json";
                payload_ = json_descr["payload"].dump();
                header_["Content-Length"]= std::to_string(payload_.size());
            }else{
                throw std::runtime_error("Invalid format for attribute:payload in http connector");
            }
        }else {
            payload_ = "";
        }

        try{
            authbundle_id_ = json_descr.at("authbundle_id").get<std::string>();
        }catch(json::exception){
            authbundle_id_ = "";
        }

        try{
            request_freq_limit_ = json_descr.at("request_freq_limit").get<int>();
        }catch(json::exception){
            request_freq_limit_ = 0;
        }
    }

    void connect()override{
        if(authbundle_id_ != ""){
            parse_authbundle();
        }
        curl = curl_easy_init();
        if(!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }

    void disconnect()override{
        if(curl) {
            curl_easy_cleanup(curl);
        }
    }

    void do_send(MessageWrapper & msg_w)override{
        HttpResponse response = send_request();
        if (response.curl_code != CURLE_OK){
            throw std::runtime_error("Error while sending Http request");
        }
    }

    Message do_receive()override{
        if(request_freq_limit_ != 0){
            std::this_thread::sleep_for(std::chrono::seconds(request_freq_limit_));
        }
        HttpResponse response = send_request();
        if (response.curl_code != CURLE_OK){
            throw std::runtime_error("Error while sending Http request");
        }
        return Message(response.resp_str, "http");
    }
};


json http_connector_schema_ = {
    "http", {
        {"type", {
            {"type", "string"},
            {"enum", {"http"}},
            {"required", true}
        }},
        {"authbundle_id", {
            {"type", "string"},
            {"required", true}
        }},
        {"url", {
            {"type", "string"},
            {"required", true}
        }},
        {"https_verify_cert", {
            {"type", "boolean"},
            {"default", true},
            {"required", false}
        }},
        {"method", {
            {"type", "string"},
            {"required", true}
        }},
        {"header", {
            {"type", "string"},
            {"required", false}
        }},
        {"payload", {
            {"type", "string"},
            {"default", ""},
            {"required", false}
        }},
        {"request_freq_limit", {
            {"type", "integer"},
            {"default", 0},
            {"required", false}
        }}
    }
};


#endif  // __M2E_BRIDGE_HTTP_CONNECTOR_H__