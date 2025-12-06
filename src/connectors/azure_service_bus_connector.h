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


#ifndef __M2E_BRIDGE_AZURE_SERVICE_BUS_CONNECTOR_H__
#define __M2E_BRIDGE_AZURE_SERVICE_BUS_CONNECTOR_H__


#include <iostream>
#include <string>
#include <map>
#include <atomic>
#include <stdexcept>
#include <regex>
#include <iomanip>

#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <curl/curl.h>

#include <Poco/Base64Encoder.h>
#include <Poco/URI.h>

#include "connector.h"
#include "database/authbundle.h"


class ServiceBusConnector: public Connector{
    std::string connection_string_;
    std::string endpoint_;
    std::string access_key_name_;
    std::string access_key_;
    std::string entity_path_;

    std::string sas_token_;
    std::time_t sas_token_expiry_;
    int expire_in_;
    std::string resource_uri_;
    bool is_topic_;
    std::string subscription_name_;
    bool delete_after_processing_;

    CURL * curl_;
    CURLcode res_ = CURLE_OK;

    void parse_authbundle(){
        AuthbundleTable db;
        AuthBundle ab;
        bool res = db.get(authbundle_id_, ab);
        if(res){
            if(ab.service_type != ServiceType::AZURE){
                throw std::runtime_error("Incompatiable authbundle service type");
            }
            if(ab.auth_type != AuthType::ACCESS_KEY){
                throw std::runtime_error("Incompatiable authbundle auth type");
            }
            connection_string_ = ab.password;
        }else{
            throw std::runtime_error("Not able to retrieve authbundle");
        }
    }

    void parse_connection_string(){
        std::regex endpoint_regex("Endpoint=([^;]+);");
        std::regex keyname_regex("SharedAccessKeyName=([^;]+);");
        std::regex key_regex("SharedAccessKey=([^;]+)");

        std::smatch match;

        if(std::regex_search(connection_string_, match, endpoint_regex)){
            endpoint_ = match[1];
        }else{
            throw std::runtime_error("endpoint could not found");
        }

        if(std::regex_search(connection_string_, match, keyname_regex)){
            access_key_name_ = match[1];
        }else{
            throw std::runtime_error("access key name could not found");
        }

        if(std::regex_search(connection_string_, match, key_regex)){
            access_key_ = match[1];
        }else{
            throw std::runtime_error("access key could not found");
        }
    }

    std::string base64_encode(unsigned const char * input, int length){
        std::stringstream ss;
        Poco::Base64Encoder encoder(ss);
        encoder.rdbuf()->setLineLength(0);
        encoder.write(reinterpret_cast<char const *>(input), length);
        encoder.close();

        return ss.str();
    }

    string url_encode(string const & value){
        std::string encoded;
        Poco::URI::encode(value, "=", encoded);
        return encoded;
    }

    std::string generate_sas_token(const std::string& resource_uri){
        std::time_t expiry = std::time(nullptr) + expire_in_;

        std::ostringstream string_to_sign;
        string_to_sign << url_encode(resource_uri) << "\n" << expiry;

        unsigned char hmac[EVP_MAX_MD_SIZE];
        unsigned int hmac_length = 0;

        HMAC(EVP_sha256(), access_key_.c_str(), access_key_.length(),
            reinterpret_cast<const unsigned char*>(string_to_sign.str().c_str()), string_to_sign.str().length(),
            hmac, &hmac_length);

        std::string signature = base64_encode(hmac, hmac_length);

        std::string url_encoded_signature = url_encode(signature);

        std::ostringstream sas_token;
        sas_token << "SharedAccessSignature sr=" << url_encode(resource_uri)
                  << "&sig=" << url_encoded_signature
                  << "&se=" << expiry
                  << "&skn=" << access_key_name_;

        return sas_token.str();
    }

    void refresh_sas_token(){
        if(std::time(nullptr) >= sas_token_expiry_){
            sas_token_ = generate_sas_token(resource_uri_);
        }
    }

    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp){
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

public:
    ServiceBusConnector(std::string pipeid, ConnectorMode mode, json const & json_descr):
            Connector(pipeid, mode, json_descr)
    {
        if(authbundle_id_.empty()){
            throw std::runtime_error("authbundle_id cannot be null for azure service bus connector");
        }else{
            parse_authbundle();
        }

        try{
            entity_path_ = json_descr.at("entity_path").get<std::string>();
        }catch(json::exception){
            throw std::runtime_error("entity_path cannot be null for azure service bus connector");
        }

        try{
            expire_in_ = json_descr.at("expire_in").get<std::time_t>();
        }catch(json::exception){
            expire_in_ = 3600;  // Default one hour
        }

        try{
            is_topic_ = json_descr.at("is_topic").get<bool>();
        }catch(json::exception){
            throw std::runtime_error("is_topic cannot be null for azure service bus connector");
        }

        if(is_topic_ && mode_ == ConnectorMode::IN){
            try{
                subscription_name_ = json_descr.at("subscription_name").get<std::string>();
            }catch(json::exception){
                throw std::runtime_error("subscription_name cannot be null for azure service bus connector_in");
            }
        }

        if(json_descr.contains("delete_after_processing")){
            delete_after_processing_ = json_descr.at("delete_after_processing").get<bool>();
        }else if(is_topic_){
            delete_after_processing_ = false;
        }else{
            delete_after_processing_ = true;
        }
    }

    void do_connect()override{
        try{
            parse_connection_string();
        }catch(std::exception& e){
            throw std::runtime_error("Incompatiable connection string");
        }

        resource_uri_ = endpoint_ + entity_path_;
        if(resource_uri_.find("sb://") == 0){
            resource_uri_.replace(0, 5, "https://");
        }
        try{
            sas_token_ = generate_sas_token(resource_uri_);
        }catch(std::exception& e){
            throw std::runtime_error("Error generating SAS token");
        }

        curl_ = curl_easy_init();
        if(!curl_) {
            throw std::runtime_error("Failed to initialize CURL");
        }

        std::cout << "Connected to Azure Service Bus: " << resource_uri_ << std::endl;
    }

    void do_send(MessageWrapper & msg_w)override{
        std::string payload = msg_w.msg().get_raw();
        std::string content_type = "text/plain; charset=utf-8";

        // Set CURL options
        curl_easy_setopt(curl_, CURLOPT_URL, (resource_uri_ + "/messages").c_str());
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload.c_str());

        // Set headers
        struct curl_slist* headers = nullptr;
        refresh_sas_token();
        headers = curl_slist_append(headers, ("Authorization: " + sas_token_).c_str());
        headers = curl_slist_append(headers, ("Content-Type: " +  content_type).c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        res_ = curl_easy_perform(curl_);
        if(res_ != CURLE_OK){
            throw std::runtime_error(curl_easy_strerror(res_));
        }else{
            curl_slist_free_all(headers);  // Clean up
        }
    }

    Message const do_receive()override{
        std::string receive_url = resource_uri_;
        if(is_topic_){
            receive_url += "/subscriptions/" + subscription_name_ + "/messages/head?api-version=2013-08";
        }else{
            receive_url += "/messages/head?api-version=2013-08";
        }
        // Set CURL options
        curl_easy_setopt(curl_, CURLOPT_URL, receive_url.c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);

        // Set headers
        struct curl_slist* headers = nullptr;
        refresh_sas_token();
        headers = curl_slist_append(headers, ("Authorization: " + sas_token_).c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        // Store the response
        std::string response;
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);

        // Perform the request
        res_ = curl_easy_perform(curl_);
        if(res_ != CURLE_OK){
            throw std::runtime_error(curl_easy_strerror(res_));
        }

        if(response.empty()){
            throw std::out_of_range("No messages");
        }

        if(delete_after_processing_){
            curl_easy_setopt(curl_, CURLOPT_URL, receive_url.c_str());
            curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");

            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
            res_ = curl_easy_perform(curl_);
            if(res_ != CURLE_OK){
                throw std::runtime_error(curl_easy_strerror(res_));
            }
        }

        // Clean up
        curl_slist_free_all(headers);

        return Message(response, msg_format_, entity_path_);
    }

    void do_disconnect()override{
        if(curl_){
            curl_easy_cleanup(curl_);
        }
    }

    static pair<string, json> get_schema(){
        json schema = Connector::get_schema();
        schema.merge_patch({
           {"authbundle_id", {
                {"options", {
                    {"filter", {
                        {"key", "service_type"},
                        {"value", "azure"}
                    }}
                }}
            }},
            {"entity_path", {
                {"type", "string"},
                {"required", true}
            }},
            {"expire_in", {
                {"type", "integer"},
                {"default", 3600},
                {"required", false}
            }},
            {"is_topic", {
                {"type", "boolean"},
                {"required", true}
            }},
            {"subscription_name", {
                {"type", "string"},
                {"required", {{"key", "mode"}, {"value", "in"}}}
            }},
            {"delete_after_processing", {
                {"type", "boolean"},
                {"required", false}
            }}
        });
        return {"azure_service_bus", schema};
    }
};


#endif  // __M2E_BRIDGE_AZURE_SERVICE_BUS_CONNECTOR_H__