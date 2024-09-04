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
    PubSubConnector(nlohmann::json json_descr, std::string type) :Connector(json_descr, type)  {
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
        if (connector_type_ == "connector_in" ){
            if(json_descr["subscription_id"].is_null()){
                throw std::runtime_error("Subscription ID cannot be null for pubsub connector_in\n");
            }
            subscription_id_ = json_descr["subscription_id"];
        }
          if (connector_type_ == "connector_out" ){
            if(json_descr["topic_id"].is_null()){
                throw std::runtime_error("Topic ID cannot be null for pubsub connector_out\n");
            }
            topic_id_ = json_descr["topic_id"];
        }
    }

    void connect() {

        if (connector_type_ == "connector_in" ){
            subscriber_ptr_ = new pubsub::Subscriber(pubsub::MakeSubscriberConnection(
                pubsub::Subscription(project_id_, subscription_id_))
            );
        }
        if (connector_type_ == "connector_out" ){
            publisher_ptr_ = new pubsub::Publisher(
                pubsub::MakePublisherConnection(pubsub::Topic(project_id_, topic_id_))
            );
        }
    }
    void disconnect() {}
    MessageWrapper* receive() {
        // try{
        //     auto session =
        // subscriber.Subscribe([&](pubsub::Message const& m, pubsub::AckHandler h) {
        //     std::cout << "Received message " << m << "\n";
        //     std::move(h).ack();
        // });

        //     std::cout << "Waiting for messages on " + subscription_id_ + "...\n";

        //     // Blocks until the timeout is reached.
        //     auto result = session.wait_for(kWaitTimeout);
        //     if (result == std::future_status::timeout) {
        //         std::cout << "timeout reached, ending session\n";
        //         session.cancel();
        //     }

        //     return 0;


        // }catch (google::cloud::Status const& status) {
        //     std::cerr << "google::cloud::Status thrown: " << status << "\n";
        //     return 1;
        // }
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