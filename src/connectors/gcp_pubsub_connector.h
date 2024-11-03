#ifndef __M2E_BRIDGE_GCP_PUBSUB_CONNECTOR_H__
#define __M2E_BRIDGE_GCP_PUBSUB_CONNECTOR_H__


#include <string>
#include <map>
#include <iostream>
#include <utility>
#include <stdexcept>
#include <cstdlib>
#include <regex>
#include <fstream>
#include <iterator>

#include "google/cloud/pubsub/publisher.h"
#include "google/cloud/pubsub/subscriber.h"
#include "google/cloud/pubsub/message.h"
#include "google/cloud/pubsub/admin/topic_admin_client.h"
#include "google/cloud/pubsub/topic.h"
#include "google/cloud/status.h"
#include "google/cloud/pubsub/admin/subscription_admin_client.h"
#include "google/pubsub/v1/pubsub.pb.h"
#include "google/cloud/options.h"
#include "google/cloud/credentials.h"

#include "connector.h"
#include "database.h"


namespace gcp {

namespace pubsub = ::google::cloud::pubsub;
namespace gcloud = ::google::cloud;

using std::string;
using std::map;


class PubSubConnector: public Connector{
private:
    string project_id_;
    string topic_id_;
    string subscription_id_;
    string authbundle_id_;
    string service_key_data_;

    pubsub::Publisher* publisher_ptr_;
    pubsub::Subscriber* subscriber_ptr_;
    gcloud::Options auth_options_;
    gcloud::pubsub_admin::TopicAdminClient *topic_admin_;
    gcloud::pubsub_admin::SubscriptionAdminClient *sub_admin_;

    map<string, std::pair<bool,string>> attributes_;

public:
    PubSubConnector(std::string pipeid, ConnectorMode mode, json const & json_descr):
            Connector(pipeid, mode, json_descr){
        try{
            authbundle_id_ = json_descr.at("authbundle_id").get<string>();
        }catch(json::exception){
            throw std::runtime_error("authbundle_id cannot be null for pubsub connector");
        }
        try{
            project_id_ = json_descr.at("project_id").get<string>();
        }catch(json::exception){
            throw std::runtime_error("Project ID cannot be null for pubsub connector");
        }

        if(mode_ == ConnectorMode::IN){
            // For connector IN, either topic Id or subscription Id should be provided
            // If subscription Id is provided without topic Id, it is assumed that the subscription
            // is already created.
            // If topic Id is provided without subscription Id, we try to create a subscription for
            // that topic whose name is <pipeid>-<topicid>-sub or resuse subscription if
            // it's already present.
            // If both topic Id and subscription Id are given, we try to create subscription
            // with given subscription Id for that topic or else resuse subscription
            // if it's already present
            try{
                subscription_id_ = json_descr.at("subscription_id").get<string>();
            }catch(json::exception){
                subscription_id_ = "";
            }
            try{
                topic_id_ = json_descr.at("topic_id").get<string>();
            }catch(json::exception){
                topic_id_ = "";
            }
            if (topic_id_ == "" && subscription_id_ == ""){
                throw std::runtime_error(
                    "Either Topic ID or Subscription ID must be present for pubsub connector_in"
                );
            }
            if(subscription_id_ == ""){
                subscription_id_ = pipeid_ + "-" + topic_id_ + "-sub";
            }
        }else if(mode_ == ConnectorMode::OUT){
            try{
                topic_id_ = json_descr.at("topic_id").get<string>();
            }catch(json::exception){
                throw std::runtime_error("Topic ID cannot be null for pubsub connector_out");
            }
        }else{
            throw std::runtime_error("Unsupported connector mode!");
        }

        if(json_descr.contains("attributes")){
            std::smatch match;
            std::regex pattern("\\{\\{(.*?)\\}\\}");

            json const & attributes = json_descr["attributes"];
            for(auto it = attributes.begin(); it != attributes.end(); ++it){
                string v = (*it).get<string>();
                bool is_dynamic = std::regex_search(v.cbegin(), v.cend(), match, pattern);
                attributes_[it.key()] = std::pair(is_dynamic, v);
            }
        }

    }

    void connect()override{
        parse_authbundle();
        auth_options_ = gcloud::Options{}.set<gcloud::UnifiedCredentialsOption>(
            gcloud::MakeServiceAccountCredentials(service_key_data_));
        topic_admin_ = new gcloud::pubsub_admin::TopicAdminClient(
            gcloud::pubsub_admin::MakeTopicAdminConnection(auth_options_)
        );
        sub_admin_ = new gcloud::pubsub_admin::SubscriptionAdminClient(
                    gcloud::pubsub_admin::MakeSubscriptionAdminConnection(auth_options_));
        if(mode_ == ConnectorMode::IN){
            // We always have subscription_id here, either from config or created in the
            // constructor
            auto subscription = pubsub::Subscription(project_id_, subscription_id_);
            auto resp = sub_admin_->GetSubscription(subscription.FullName());
            if(resp.status().code() == google::cloud::StatusCode::kNotFound){
                if(topic_id_ == ""){
                    throw std::runtime_error(
                        "Subscription doesn't exist and topic_id is not provided to create one"
                    );
                }
                create_topic(); // make sure topic exist
                std::string subscription_url = (
                    "projects/" + project_id_ + "/subscriptions/" + subscription_id_
                );
                std::string topic_url = "projects/" + project_id_ + "/topics/" + topic_id_;
                google::pubsub::v1::PushConfig push_config;
                std::int32_t ack_deadline_seconds = 10;
                auto create_sub_resp = sub_admin_->CreateSubscription(
                    subscription_url, topic_url, push_config, ack_deadline_seconds
                );
                if (!create_sub_resp &&
                        create_sub_resp.status().code() != gcloud::StatusCode::kAlreadyExists){
                    std::cerr << "Error creating subscription: "<<create_sub_resp.status()<<"\n";
                    throw std::runtime_error("Error creating subscription");
                }
            }

            subscriber_ptr_ = new pubsub::Subscriber(pubsub::MakeSubscriberConnection(
                pubsub::Subscription(project_id_, subscription_id_),
                ::google::cloud::Options{}
                    .set<pubsub::MaxConcurrencyOption>(1)
                    .set<::google::cloud::GrpcBackgroundThreadPoolSizeOption>(2)
                    .set<gcloud::UnifiedCredentialsOption>(
                        gcloud::MakeServiceAccountCredentials(service_key_data_))
            ));
        }else if(mode_ == ConnectorMode::OUT){
            create_topic();
            publisher_ptr_ = new pubsub::Publisher(pubsub::MakePublisherConnection(
                pubsub::Topic(project_id_, topic_id_),
                ::google::cloud::Options{}
                    .set<pubsub::MaxConcurrencyOption>(1)
                    .set<::google::cloud::GrpcBackgroundThreadPoolSizeOption>(2)
                    .set<gcloud::UnifiedCredentialsOption>(
                        gcloud::MakeServiceAccountCredentials(service_key_data_))
            ));
        }
    }

    Message do_receive()override{
        try{
            auto opts = google::cloud::Options{}
                .set<pubsub::RetryPolicyOption>(pubsub::LimitedTimeRetryPolicy(
                                              std::chrono::milliseconds(500))
                                              .clone());
            google::cloud::StatusOr<google::cloud::pubsub::PullResponse> response;
            while(is_active_){
                response = subscriber_ptr_->Pull(opts);
                if(response) break;
            }
            if(! response){
                throw std::out_of_range("No messages");
            }

            string msg_text = response->message.data();
            std::cout << "Received message " << msg_text << "\n";
            std::move(response->handler).ack();
            return Message(msg_text, subscription_id_);
        }catch (google::cloud::Status const& status) {
            std::cerr << "google::cloud::Status thrown: " << status << "\n";
            throw std::runtime_error("Error pulling messages from gcp pubsub");
        }
    }

    void do_send(MessageWrapper & msg_w)override{
        try{
            auto mb = pubsub::MessageBuilder{}.SetData(msg_w.msg().get_raw());
            for(auto const & attribute : attributes_){
                if(attribute.second.first){
                    string av = derive_attribute(msg_w, attribute.second.second);
                    mb.InsertAttribute(attribute.first, av);
                }else{
                    mb.InsertAttribute(attribute.first, attribute.second.second);
                }
            }

            auto id = publisher_ptr_->Publish(std::move(mb).Build()).get();
            if(!id) throw std::move(id).status();
        }
        catch (google::cloud::Status const& status) {
            throw std::runtime_error("Unable to publish to gcp pub/sub");
        }
    }

    string derive_attribute(MessageWrapper & msg_w, string const & atemplate){
        using namespace std;

        regex pattern_variable("\\{\\{(.*?)\\}\\}");
        regex pattern_expression("topic\\[(.*?)\\]");

        string attribute = atemplate;
        auto pos = attribute.cbegin();
        smatch match1;
        while(regex_search(pos, attribute.cend(), match1, pattern_variable)){
            string ve = match1[1].str();
            smatch match2;
            if(regex_search(ve.cbegin(), ve.cend(), match2, pattern_expression)){
                int topic_level = stoi(match2[1].str());
                string vvalue = msg_w.msg().get_topic_level(topic_level);
                unsigned int i = (pos - attribute.cbegin());
                attribute.replace(i + match1.position(), match1.length(), vvalue);
                // Restore iterator after string modification
                pos = attribute.cbegin() + i + match1.position() + vvalue.size();
            }else{
                pos = pos + match1.position() + match1[0].str().size();
            }
        }
        return attribute;
    }

private:
    void create_topic(){
        std::string topic_url = "projects/" + project_id_ + "/topics/" + topic_id_;
        auto res = topic_admin_->CreateTopic(topic_url);
        // Throw error if topic doesn't already exists
        if (! res && res.status().code() != gcloud::StatusCode::kAlreadyExists){
            std::cerr << "Error creating topic: "<<res.status()<<std::endl;
            throw std::runtime_error("Error creating topic");
        }
    }

    void parse_authbundle(){
        Database db;
        AuthBundle ab;
        bool res = db.retrieve_authbundle(authbundle_id_, ab);
        if(res){
            if(ab.connector_type != ConnectorType::GCP_PUBSUB){
                throw std::runtime_error("Incompatiable authbundle connector type");
            }
            if(ab.auth_type != AuthType::SERVICE_KEY){
                throw std::runtime_error("Incompatiable authbundle auth type");
            }
            service_key_data_ = ab.keydata;
        }
        else{
            throw std::runtime_error("Not able to retreive bundle");
        }
    }
};

};


#endif