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


#ifndef __M2E_BRIDGE_EMAIL_CONNECTOR_H__
#define __M2E_BRIDGE_EMAIL_CONNECTOR_H__

#include <string>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <vector>
#include <regex>

#include <curl/curl.h>

#include "connector.h"
#include "database/authbundle.h"

const int  SMTP_PORT = 587;  // Default SMTP port
const int IMAP_PORT = 993;  // Default IMAP port


class EmailConnector: public Connector {
    friend class EmailConnectorTests;
private:
    std::string smtp_server_;
    std::string imap_server_;
    std::string address_;
    int smtp_port_;
    int imap_port_;
    std::string username_;  // Email address to send mail from
    std::string password_;  // Password for pluraf mail address

    std::string subject_;
    bool search_ = false;

    CURL * curl;
    CURLcode res = CURLE_OK;

    void parse_authbundle(){
        AuthbundleTable db;
        AuthBundle ab;
        bool res = db.get(authbundle_id_, ab);
        if(res){
            username_ = ab.username;
            password_ = ab.password;
        }
        else{
            throw std::runtime_error("Not able to retreive bundle");
        }
    }

    struct upload_status{
        int lines_read;
        std::vector<std::string>* email_content;
    };

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

public:
    EmailConnector(std::string pipeid, ConnectorMode mode, json const & json_descr):
            Connector(pipeid, mode, json_descr)
    {
        if(authbundle_id_.empty()){
            throw std::runtime_error("authbundle_id cannot be null for email connector");
        }else{
            parse_authbundle();
        }

        try{
            address_ = json_descr.at("address").get<std::string>();
        }
        catch(json::exception const & e)
        {
            //throw std::runtime_error("Address cannot be null for email connector");
            throw std::runtime_error(e.what());
        }

        if(mode_ == ConnectorMode::OUT){
            try{
                smtp_server_ = json_descr.at("smtp_server").get<std::string>();
            }catch(json::exception){
                throw std::runtime_error("Smtp server cannot be null for email connector_out");
            }

            try{
                smtp_port_ = json_descr.at("smtp_port").get<int>();
            }catch(json::exception){
                smtp_port_ = SMTP_PORT;
            }
        }else if(mode_ == ConnectorMode::IN){
            try{
                imap_server_ = json_descr.at("imap_server").get<std::string>();
            }catch(json::exception){
                throw std::runtime_error("Imap server cannot be null for email connector_in");
            }

            try{
                imap_port_ = json_descr.at("imap_port").get<int>();
            }catch(json::exception){
                imap_port_ = IMAP_PORT;
            }

            if(json_descr.contains("subject")){
                search_ = true;
                subject_ = json_descr.at("subject").get<std::string>();
            }
            }else{
                throw std::runtime_error("Unsupported connector mode!");
            }
    }

    void send_email( const std::string& subject, const std::string& body )
    {
        upload_status upload_ctx;
        upload_ctx.lines_read = 0;

        std::vector<std::string> email_content = {
            "To: " + address_ + "\r\n",
            "From: " + username_ + "\r\n" ,
            "Subject: " + subject + "\r\n" ,
            "\r\n",
            body + "\r\n",
        };
        upload_ctx.email_content = &email_content;

        if( curl )
        {
            struct curl_slist* recipients = nullptr;

            curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + username_ + ">").c_str());
            recipients = curl_slist_append(recipients, ("<" + address_ + ">").c_str());
            curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

            curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
            curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
            curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

            res = curl_easy_perform(curl);

            if(res != CURLE_OK){
                std::cerr << curl_easy_strerror(res) << std::endl;
            }else{
                std::cout << "Email sent successfully" << std::endl;
            }

            curl_slist_free_all(recipients);
        }
    }

    void do_connect( )override
    {
        curl = curl_easy_init();
        if(!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }

        std::string url;
        if( mode_ == ConnectorMode::OUT )
        {
            url = "smtp://" + smtp_server_ + ":" + std::to_string(smtp_port_);
        }
        else
        {
            url = "imaps://" + imap_server_ + ":" + std::to_string(imap_port_) + "/INBOX";
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, username_.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password_.c_str());

        // Enable TLS
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        // Set timeout
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            std::cerr << "Connection failed: " << curl_easy_strerror(res) << std::endl;
        }else{
            std::cout << "Connection success" << std::endl;
        }
    }

    void do_send( MessageWrapper & msg_w )override
    {
        std::string subject = msg_w.msg().get_topic();
        std::string body = msg_w.msg().get_raw();

        send_email(subject, body);
    }

    void do_disconnect()override{
        std::cout << "EmailConnector is disconnecting..." << std::endl;

        if(curl) {
            curl_easy_cleanup(curl);
        }
    }

    static pair<string, json> get_schema(){
        json schema = Connector::get_schema();
        schema.merge_patch({
           {"authbundle_id", {
                {"options", {
                    {"filter", {
                        {"key", "service_type"},
                        {"value", "email"}
                    }}
                }}
            }},
            {"address", {
                {"type", "string"},
                {"required", true}
            }},
            {"smtp_server", {
                {"type", "string"},
                {"required", {{"in", false}, {"out", true}}}
            }},
            {"smtp_port", {
                {"type", "integer"},
                {"default", 587},
                {"required", false}
            }},
            {"imap_server", {
                {"type", "string"},
                {"required", {{"in", true}, {"out", false}}}
            }},
            {"imap_port", {
                {"type", "integer"},
                {"default", 993},
                {"required", false}
            }},
            {"subject", {
                {"type", "string"},
                {"required", false}
            }}
        });
        return {"email", schema};
    }
};


#endif  // __M2E_BRIDGE_EMAIL_CONNECTOR_H__