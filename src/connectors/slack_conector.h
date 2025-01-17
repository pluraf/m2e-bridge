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
                throw std::runtime_error("Incompatiable authbundle service type");
            }
            // Need different passwords for different connector modes
            switch (mode_){
                case ConnectorMode::IN:
                    oauth_token_ = ab.password;
                    break;
                case ConnectorMode::OUT:
                    webhook_url_ = ab.password;
                    break;
                default:
                    throw std::runtime_error("Unsupported connector mode!");
            }

        }else{
            throw std::runtime_error("Not able to retrieve authbundle!");
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
            parse_authbundle();
        }catch(json::exception&){
            throw std::runtime_error("authbundle_id cannot be null for Slack connector");
        }

        if(mode_ == ConnectorMode::IN){
            try {
                channel_id_ = json_descr.at("channel_id").get<std::string>();
            }catch(json::exception&){
                throw std::runtime_error("channel_id cannot be null for Slack connector_out");
            }
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
        curl_ = curl_easy_init();
        if(!curl_){
            throw std::runtime_error("Failed to initialize CURL");
        }
        std::cout << "SlackConnector connected successfully" << std::endl;
    }

    void do_send(MessageWrapper& msg_w) override{
        send_message(msg_w.msg().get_raw());
    }

    Message do_receive()override{
        std::string response_data;

        // Prepare URL
        std::string url = "https://slack.com/api/conversations.history";
        std::string post_data = "{\"channel\":\"" + channel_id_ + "\", \"limit\": 1}";

        // Set CURL options
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, post_data.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, post_data.size());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + oauth_token_).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_data);

        // Perform the request
        CURLcode res = curl_easy_perform(curl_);
        curl_slist_free_all(headers);

        if(res != CURLE_OK){
            throw std::runtime_error("Failed to fetch Slack messages: " + std::string(curl_easy_strerror(res)));
        }

        // Parse JSON response
        json json_response = json::parse(response_data);
        if(!json_response.contains("ok") || !json_response["ok"].get<bool>()){
            throw std::runtime_error("Error fetching messages: " + json_response.dump());
        }

        // Extract the last message
        std::string message_;
        if(!json_response["messages"].empty()){
            message_ = json_response["messages"][0].at("text");
        }else{
            message_ = "";
        }

        return Message(message_, "Slack");
    }

    void disconnect() override{
        if(curl_){
            curl_easy_cleanup(curl_);
        }
        std::cout << "SlackConnector disconnected" << std::endl;
    }
};


json slack_connector_schema_ = {
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
