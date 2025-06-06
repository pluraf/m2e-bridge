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


#ifndef __M2E_BRIDGE_MQTT_CONNECTOR_H__
#define __M2E_BRIDGE_MQTT_CONNECTOR_H__


#include <iostream>
#include <string>
#include <atomic>
#include <stdexcept>
#include <random>
#include <regex>

#include <fmt/core.h>
#include <jwt-cpp/jwt.h>
#include "mqtt/async_client.h"
#include "Poco/String.h"

#include "connector.h"
#include "database/authbundle.h"
#include "substitutions/subs.hpp"

const int QOS=1;
const int N_RETRY_ATTEMPTS = 10;

const auto TIMEOUT = std::chrono::seconds(10);


using std::string;

inline std::string generate_random_id(size_t length) {
    const string CHARACTERS
        = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuv"
          "wxyz0123456789";

    // Create a random number generator
    std::random_device rd;
    std::mt19937 generator(rd());

    // Create a distribution to uniformly select from all
    // characters
    std::uniform_int_distribution<> distribution(
        0, CHARACTERS.size() - 1);

    // Generate the random string
    string random_string;
    for (int i = 0; i < length; ++i) {
        random_string
            += CHARACTERS[distribution(generator)];
    }

    return random_string;
}

class ActionListener : public virtual mqtt::iaction_listener
{
    std::string name_;

    void on_failure(const mqtt::token& tok) override {
        std::cout << name_ << " failure";
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        std::cout << std::endl;
    }

    void on_success(const mqtt::token& tok) override {
        if (tok.get_message_id() != 0)
            std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
        auto top = tok.get_topics();
        if (top && !top->empty())
            std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." << std::endl;
        std::cout << std::endl;
    }

public:
    ActionListener(const std::string& name) : name_(name) {}
};


class MqttConnector: public Connector{
public:
    MqttConnector(std::string pipeid, ConnectorMode mode, json const & json_descr):
            Connector(pipeid, mode, json_descr){
        // server
        try{
            server_ = json_descr.at("server").get<string>();
        }catch(json::exception){
            server_ = "tcp://127.0.0.1:1884";
        }
        // version
        try{
            auto version = Poco::trim(json_descr.at("version").get<string>());
            if(version == "5"){
                mqtt_version_ = MQTTVERSION_5;
            }else if(version == "3.11"){
                mqtt_version_ = MQTTVERSION_3_1_1;
            }
        }catch(json::exception){
            mqtt_version_ = MQTTVERSION_5;
        }
        // topic
        try{
            topic_template_ = json_descr.at("topic").get<string>();
        }catch(json::exception){
           throw std::runtime_error("Topic cannot be null for mqtt connector");
        }
        // client_id
        try{
            client_id_ = json_descr.at("client_id").get<string>();
        }catch(json::exception){
            client_id_ = generate_random_id(10);
        }
        // retry_attempts
        try{
            n_retry_attempts_ = json_descr.at("retry_attempts").get<int>();
        }catch(json::exception){
            n_retry_attempts_ = N_RETRY_ATTEMPTS;
        }
        // qos
        try{
            qos_ = json_descr.at("qos").get<int>();
        }catch(json::exception){
            qos_ = QOS;
        }
        // verify_server_hostname
        try{
            verify_server_hostname_ =
                json_descr.at("verify_server_hostname").get<string>() == "yes";
        }catch(json::exception){
            // default value is true
        }
        // verify_server_certificate
        try{
            verify_server_certificate_ =
                json_descr.at("verify_server_certificate").get<string>() == "yes";
        }catch(json::exception){
            // default value is true
        }
        // ca_certificate
        try{
            ca_certificate_file_ = json_descr.at("ca_certificate").get<string>();
        }catch(json::exception){
            // default value is ""
        }

        std::smatch match;
        std::regex pattern("\\{\\{(.*?)\\}\\}");
        is_topic_template_ = std::regex_search(
            topic_template_.cbegin(), topic_template_.cend(), match, pattern
        );

        // Create MQTT Client
        client_ptr_ = std::make_shared<mqtt::async_client>(
            server_, client_id_, mqtt::create_options(mqtt_version_), nullptr
        );
        // Install the callback(s) before connecting.
        callback_ptr_ = std::make_unique<Callback>(this);
        client_ptr_->set_callback(* callback_ptr_);
    }

    void do_connect()override{
        msg_queue_ = std::make_unique<mqtt::thread_queue<mqtt::message>>(1000);
        conn_opts_.set_clean_session(false);
        conn_opts_.set_mqtt_version(mqtt_version_);
        if(! authbundle_id_.empty()){
            parse_authbundle();
        }

        if(server_.starts_with("ssl") || server_.starts_with("tls")){
            mqtt::ssl_options sslopts;
            sslopts.set_verify(verify_server_hostname_);
            sslopts.set_enable_server_cert_auth(verify_server_certificate_);
            if (! ca_certificate_file_.empty()) {
                sslopts.set_trust_store(gc.get_ca_storage() + ca_certificate_file_);
            }
            conn_opts_.set_ssl(sslopts);
        }
        auto token = client_ptr_->connect(conn_opts_, nullptr, * callback_ptr_);
        if(! token->wait_for(5000)) throw std::runtime_error("MQTT Connection Timeout!");
    }

    void do_disconnect()override{
        stop();
        if(client_ptr_->is_connected()){
            auto token = client_ptr_->disconnect();
            token->wait();
            if(! token->wait_for(5000)) throw std::runtime_error("MQTT Disconnection Timeout!");
        }
    }

    void do_send(MessageWrapper & msg_w)override{
        string derived_topic;
        if(is_topic_template_){
            derived_topic = derive_topic(msg_w);
        }
        string const & topic = is_topic_template_ ? derived_topic : topic_template_;
        try {
            mqtt::delivery_token_ptr pubtok;
            pubtok = client_ptr_->publish(
                topic,
                msg_w.msg().get_raw().c_str(),
                msg_w.msg().get_raw().length(),
                qos_,
                false);
            pubtok->wait_for(TIMEOUT);
        }
        catch (const mqtt::exception& exc) {
            std::cerr << exc << std::endl;
            throw std::runtime_error("Unable to send message to MQTT server");
        }
    }

    Message const do_receive()override{
        mqtt::message mqtt_msg;
        msg_queue_->get(&mqtt_msg);  // blocking call
        return Message(mqtt_msg.get_payload(), mqtt_msg.get_topic());
    }

    void do_stop()override{
        msg_queue_->exit_blocking_calls();
    }

    std::string derive_topic(MessageWrapper & msg_w){
        return std::get<string>(SubsEngine(msg_w.msg(), msg_w.get_metadata(), msg_w.msg().get_attributes()).substitute(topic_template_));
    }

    static pair<string, json> get_schema(){
        json schema = Connector::get_schema();
        schema.merge_patch({
            {"authbundle_id", {
                {"options", {
                    {"filter", {
                        {"key", "service_type"},
                        {"value", "mqtt"}
                    }}
                }}
            }},
            {"server", {
                {"type", "string"},
                {"default", "tcp://mqtt.iotplan.io:1883"},
                {"required", false}
            }},
            {"version", {
                {"type", "string"},
                {"options", {"5", "3.11"}},
                {"default", "5"},
                {"required", false}
            }},
            {"topic", {
                {"type", "string"},
                {"required", true}
            }},
            {"client_id", {
                {"type", "string"},
                {"required", false}
            }},
            {"retry_attempts", {
                {"type", "integer"},
                {"default", 10},
                {"required", false}
            }},
            {"qos", {
                {"type", "integer"},
                {"default", 1},
                {"required", false}
            }},
            {"ca_certificate", {
                {"type", "string"},
                {"options", {
                    {"url", "api/ca/"},
                    {"key", "id"}
                }},
                {"required", false}
            }},
            {"verify_server_hostname", {
                {"type", "string"},
                {"options", {"yes", "no"}},
                {"default", "yes"},
                {"required", false}
            }},
            {"verify_server_certificate", {
                {"type", "string"},
                {"options", {"yes", "no"}},
                {"default", "yes"},
                {"required", false}
            }}
        });
        return {"mqtt", schema};
    }

private:
    void parse_authbundle(){
        AuthbundleTable db;
        AuthBundle ab;
        bool res = db.get(authbundle_id_, ab);
        if(res){
            std::string token;
            switch(ab.auth_type){
                case AuthType::PASSWORD:
                    conn_opts_.set_user_name(ab.username);
                    conn_opts_.set_password(ab.password);
                    break;
                case AuthType::JWT_ES256:
                    if(!ab.username.empty()){
                        conn_opts_.set_user_name(ab.username);
                    }
                    token = jwt::create()
                        .set_issuer("m2e-bridge")
                        .set_type("JWT")
                        .set_issued_at(std::chrono::system_clock::now())
                        .set_payload_claim("client_id", jwt::claim(std::string{client_id_}))
                        .sign(jwt::algorithm::es256( "", ab.keydata, "", ""));
                    conn_opts_.set_password(token);
                    break;
                default:
                    throw std::runtime_error("Incompatiable authbundle auth type!");
            }
        }
        else{
            throw std::runtime_error("Not able to retrieve authbundle!");
        }
    }

    class Callback;
    string server_;
    string client_id_;
    mqtt::async_client_ptr  client_ptr_;
    string topic_template_;
    bool is_topic_template_ {false};
    int n_retry_attempts_;
    int qos_;
    mqtt::connect_options conn_opts_;
    std::unique_ptr<mqtt::thread_queue<mqtt::message>> msg_queue_;
    std::unique_ptr<Callback> callback_ptr_;
    string authbundle_id_;
    int mqtt_version_ = MQTTVERSION_5;
    bool verify_server_hostname_ {true};
    bool verify_server_certificate_ {true};
    string ca_certificate_file_;

private:
    class Callback : public virtual mqtt::callback, public virtual mqtt::iaction_listener{
    public:
        Callback(MqttConnector *connector):
                nretry_(0),connector_ptr_(connector),subListener_("Subscription"){}
        // Counter for the number of connection retries
        int nretry_;
        MqttConnector* connector_ptr_;
        ActionListener subListener_;
    private:
        void reconnect() {
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            try{
                connector_ptr_->client_ptr_->connect(connector_ptr_->conn_opts_, nullptr, *this);
            }catch(const mqtt::exception& exc){
                std::cerr << "Error: " << exc.what() << std::endl;
            }
        }

        // Re-connection failure
        void on_failure(const mqtt::token& tok) override {
            // std::cerr<<"Connection attempt failed "<<std::endl;
            if(++nretry_ > connector_ptr_->n_retry_attempts_) return;
            reconnect();
        }

        // (Re)connection success
        // Either this or connected() can be used for callbacks.
        void on_success(const mqtt::token& tok) override {}

        // (Re)connection success
        void connected(const std::string& cause) override {
            std::cout << "\nConnection success"  << std::endl;

            if(connector_ptr_->mode_ == ConnectorMode::IN){
                std::cout << "\nSubscribing to topic '" << connector_ptr_->topic_template_ << "'\n"
                    << " using QoS" << connector_ptr_->qos_ << std::endl;
                try {
                    connector_ptr_->client_ptr_->subscribe(connector_ptr_->topic_template_, connector_ptr_->qos_, nullptr, subListener_);
                }
                catch (const mqtt::exception& exc) {
                    std::cerr << exc << std::endl;
                    throw std::runtime_error("Unable to subscribe");
                }
                std::cout << "Subscribed"<<std::endl;
            }

        }

        // Callback for when the connection is lost.
        // This will initiate the attempt to manually reconnect.
        void connection_lost(const std::string& cause) override {
            std::cout << "\nConnection lost" << std::endl;
            if (!cause.empty())
                std::cout << "\tcause: " << cause << std::endl;

            std::cout << "Reconnecting..." << std::endl;
            nretry_ = 0;
            reconnect();
        }

        // Callback for when a message arrives.
        void message_arrived(mqtt::const_message_ptr msg) override {
            connector_ptr_->msg_queue_->put(*msg);
        }

        void delivery_complete(mqtt::delivery_token_ptr token) override {}
    };
};


#endif  // __M2E_BRIDGE_MQTT_CONNECTOR_H__