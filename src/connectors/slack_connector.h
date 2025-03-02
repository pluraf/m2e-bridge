#ifndef __M2E_BRIDGE_SLACK_CONNECTOR_H__
#define __M2E_BRIDGE_SLACK_CONNECTOR_H__


#include <string>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <curl/curl.h>

#include "connector.h"
#include "database.h"


class SlackConnector : public Connector {
private:
    std::string webhook_url_;
    std::string oauth_token_;
    std::string authbundle_id_;
    std::string channel_id_;
    CURL* curl_;

    void parse_authbundle(){
        Database db;
        AuthBundle ab;
        bool res = db.retrieve_authbundle(authbundle_id_, ab);
        if(res){
            if(ab.service_type != ServiceType::SLACK){
                throw incompatible_dependency("Incompatiable authbundle service type");
            }
            webhook_url_ = ab.password;
        }else{
            throw missing_dependency("Not able to retrieve authbundle!");
        }
    }

    static size_t write_callback(void* ptr, size_t size, size_t nmemb, void* userp){
        std::string* data = static_cast<std::string*>(userp);
        const char* incoming_data = static_cast<char*>(ptr);
        size_t total_size = size * nmemb;

        data->append(incoming_data, total_size);

        return total_size;
    }

public:
    SlackConnector(std::string pipeid, ConnectorMode mode_, json const& json_descr)
        : Connector(pipeid, mode_, json_descr){
        try {
            authbundle_id_ = json_descr.at("authbundle_id").get<std::string>();
        }catch(json::exception){
            throw configuration_error("authbundle_id cannot be null for Slack connector");
        }

        if(mode_ == ConnectorMode::IN){
            throw configuration_error("SlackConnector does not support IN mode!");
        }
    }

    void send_message(const std::string& message){
        json payload = {
            {"text", message}
        };
        std::string payload_str = payload.dump();

        // Set CURL options
        curl_easy_setopt(curl_, CURLOPT_URL, webhook_url_.c_str());
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload_str.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload_str.size());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        CURLcode res = curl_easy_perform(curl_);
        if(res != CURLE_OK){
            throw std::runtime_error("Failed to send Slack message: " + std::string(curl_easy_strerror(res)));
        }

        curl_slist_free_all(headers);
    }

    void connect() override{
        parse_authbundle();
        curl_ = curl_easy_init();
        if(!curl_){
            throw std::runtime_error("Failed to initialize CURL");
        }
        std::cout << "SlackConnector connected successfully" << std::endl;
    }

    void do_send(MessageWrapper& msg_w) override{
        send_message(msg_w.msg().get_raw());
    }

    void disconnect() override{
        if(curl_){
            curl_easy_cleanup(curl_);
        }
        std::cout << "SlackConnector disconnected" << std::endl;
    }
};


static const json slack_connector_schema_ = {
    "slack", {
        {"type", {
            {"type", "string"},
            {"enum", {"slack"}},
            {"required", true}
        }},
        {"authbundle_id", {
            {"type", "string"},
            {"required", true}
        }},
        {"channel_id", {
            {"type", "string"},
            {"required", {{"in", false}, {"out", true}}}
        }},
    }
};


#endif  // __M2E_BRIDGE_SLACK_CONNECTOR_H__
