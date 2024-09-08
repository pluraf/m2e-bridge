#ifndef __M2E_BRIDGE_GCP_PUBSUB_CONNECTOR_H__
#define __M2E_BRIDGE_GCP_PUBSUB_CONNECTOR_H__


#include <string>
#include <map>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "google/cloud/pubsub/publisher.h"
#include "google/cloud/pubsub/subscriber.h"
#include "google/cloud/pubsub/message.h"

#include "connector.h"


namespace gcp {

namespace pubsub = ::google::cloud::pubsub;

using std::string;
using std::map;


class PubSubConnector: public Connector {
private:
    string project_id_;
    string topic_id_;
    string subscription_id_;
    string key_path_;

    const char* KEY_ENV_VARIABLE = "GOOGLE_APPLICATION_CREDENTIALS";
    pubsub::Publisher* publisher_ptr_;
    pubsub::Subscriber* subscriber_ptr_;


public:
    PubSubConnector(nlohmann::json json_descr, ConnectorMode mode):Connector(json_descr, mode){
        if(json_descr["key_path"].is_null()){
            throw std::runtime_error("key_path cannot be null for pubsub connector\n");
        }
        if(json_descr["project_id"].is_null()){
            throw std::runtime_error("Project ID cannot be null for pubsub connector\n");
        }
        key_path_ = json_descr["key_path"];
        project_id_ = json_descr["project_id"];
        if (setenv(KEY_ENV_VARIABLE, key_path_.c_str(), 1) != 0) {
            throw std::runtime_error("Error setting environment variable.\n");
        }
        if(mode_ == ConnectorMode::IN){
            if(json_descr["subscription_id"].is_null()){
                throw std::runtime_error("Subscription ID cannot be null for pubsub connector_in");
            }
            subscription_id_ = json_descr["subscription_id"];
        }else if(mode_ == ConnectorMode::OUT){
            if(json_descr["topic_id"].is_null()){
                throw std::runtime_error("Topic ID cannot be null for pubsub connector_out");
            }
            topic_id_ = json_descr["topic_id"];
        }else{
            throw std::runtime_error("Unsupported connector mode!");
        }
    }

    void connect() {
        if(mode_ == ConnectorMode::IN){
            subscriber_ptr_ = new pubsub::Subscriber(pubsub::MakeSubscriberConnection(
                pubsub::Subscription(project_id_, subscription_id_))
            );
        }else if(mode_ == ConnectorMode::OUT){
            publisher_ptr_ = new pubsub::Publisher(
                pubsub::MakePublisherConnection(pubsub::Topic(project_id_, topic_id_))
            );
        }
    }
    void disconnect() {}

    MessageWrapper* receive(){
        try{
            auto opts = google::cloud::Options{}
                .set<pubsub::RetryPolicyOption>(pubsub::LimitedTimeRetryPolicy(
                                              std::chrono::milliseconds(500))
                                              .clone());
            auto response = subscriber_ptr_->Pull(opts);
            if (!response) return NULL;
            string msg_text = response->message.data();
            std::cout << "Received message " << msg_text << "\n";
            std::move(response->handler).ack();
            Message msg(msg_text, subscription_id_);
		    return new MessageWrapper(msg);
        }catch (google::cloud::Status const& status) {
            std::cerr << "google::cloud::Status thrown: " << status << "\n";
            throw std::runtime_error("Error pulling messages from gcp pubsub\n");
        }
    }
    void send(const MessageWrapper &msg_w) {
        try{
            auto id = publisher_ptr_->Publish(
                    pubsub::MessageBuilder{}.SetData(msg_w.msg.get_msg_text()).Build()
                ).get();
            if (!id) throw std::move(id).status();
            std::cout << " published message "<< msg_w.msg.get_msg_text()<< " with id= " << *id << "\n";

        }
        catch (google::cloud::Status const& status) {
            std::cerr << "google::cloud::Status thrown: " << status << "\n";
            throw std::runtime_error("Unable to publish to gcp pub/sub\n");
        }
    }
};

};


#endif