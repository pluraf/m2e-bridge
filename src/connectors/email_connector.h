#ifndef __M2E_BRIDGE_EMAIL_CONNECTOR_H__
#define __M2E_BRIDGE_EMAIL_CONNECTOR_H__

#include <string>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <vector>

#include <curl/curl.h>

#include "connector.h"
#include "database.h"

const int  SMTP_PORT = 587;  //Default SMTP port 587


class EmailConnector: public Connector {
private:
    std::string smtp_server_;
    std::string to_;
    int smtp_port_;
    std::string username_;  // Email address to send mail from
    std::string password_;  // Password for pluraf mail address
    std::string authbundle_id_;

    CURL* curl;
    CURLcode res = CURLE_OK;

    struct upload_status {
        int lines_read;
        std::vector<std::string>* email_content;
    };

    void parse_authbundle(){
        Database db;
        AuthBundle ab;
        bool res = db.retrieve_authbundle(authbundle_id_, ab);
        if(res){
            username_ = ab.username;
            password_ = ab.password;
        }
        else{
            throw std::runtime_error("Not able to retreive bundle\n");
        }
    }

public:
    EmailConnector(
            json const & json_descr, ConnectorMode mode_, std::string pipeid
        ):Connector(json_descr, mode_, pipeid){
        if(mode_ != ConnectorMode::OUT) {
            throw std::runtime_error("Email connector only supports OUT mode");
        }

        try{
            authbundle_id_ = json_descr.at("authbundle_id").get<std::string>();
            parse_authbundle();
        }catch(json::exception){
            throw std::runtime_error("authbundle_id cannot be null for email connector\n");
        }

        try {
            to_ = json_descr.at("to").get<std::string>();
        }catch(json::exception) {
            throw std::runtime_error("Destination adress cannot be null for email connector\n");
        }

        try {
            smtp_server_ = json_descr.at("smtp_server").get<std::string>();
        }catch(json::exception) {
            throw std::runtime_error("Smtp server cannot be null for email connector\n");
        }

        try {
            smtp_port_ = json_descr.at("smtp_port").get<int>();
        }catch(json::exception) {
            smtp_port_ = SMTP_PORT;
        }

    }

    static size_t payload_source(void* ptr, size_t size, size_t nmemb, void *userp){
        struct upload_status *upload_ctx = (struct upload_status *)userp;
        const char *data;

        if((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) {
            return 0;
        }

        if(upload_ctx->lines_read < upload_ctx->email_content->size()) {
            data = upload_ctx->email_content->at(upload_ctx->lines_read).c_str();
            size_t len = strlen(data);
            memcpy(ptr, data, len);
            upload_ctx->lines_read++;

            return len;
        }

        return 0;
    }

    void send_email(const std::string& subject, const std::string& body) {
        upload_status upload_ctx;
        upload_ctx.lines_read = 0;

        std::vector<std::string> email_content = {
            "To: " + to_ + "\r\n",
            "From: " + username_ + "\r\n" ,
            "Subject: " + subject + "\r\n" ,
            "\r\n",
            body + "\r\n",
        };
        upload_ctx.email_content = &email_content;

        if(curl) {
            struct curl_slist* recipients = nullptr;

            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + username_ + ">").c_str());
            recipients = curl_slist_append(recipients, ("<" + to_ + ">").c_str());
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

            curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
            curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

            res = curl_easy_perform(curl);

            if(res != CURLE_OK) {
                std::cerr << curl_easy_strerror(res) << std::endl;
            }else{
                std::cout << "Email sent successfully" << std::endl;
            }

            curl_slist_free_all(recipients);
        }
    }

    void connect()override{
        std::cout << "EmailConnector connecting to SMTP server: " << smtp_server_ << std::endl;

        curl = curl_easy_init();
        if(!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }

        std::string url = "smtp://" + smtp_server_ + ":" + std::to_string(smtp_port_);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, username_.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password_.c_str());

        // Enable TLS
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    }

    void send(MessageWrapper & msg_w)override{
        std::string subject = "Message from M2E Bridge";
        std::string body = msg_w.msg.get_text();

        send_email(subject, body);
    }

    void disconnect()override{
        std::cout << "EmailConnector is disconnecting..." << std::endl;

        if(curl) {
            curl_easy_cleanup(curl);
        }
    }
};

#endif