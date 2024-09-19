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


namespace gcp {

namespace pubsub = ::google::cloud::pubsub;
namespace gcloud = ::google::cloud;

using std::string;
using std::map;


class PubSubConnector: public Connector {
private:
    string project_id_;
    string topic_id_;
    string subscription_id_;
    string key_path_;

    pubsub::Publisher* publisher_ptr_;
    pubsub::Subscriber* subscriber_ptr_;
    gcloud::Options auth_options_;

    map<string, std::pair<bool,string>> attributes_;

public:
    PubSubConnector(
            json const & json_descr, ConnectorMode mode, std::string pipeid
        ):Connector(json_descr, mode, pipeid){
        try{
            key_path_ = json_descr.at("key_path").get<string>();
        }catch(json::exception){
            throw std::runtime_error("key_path cannot be null for pubsub connector\n");
        }
        try{
            project_id_ = json_descr.at("project_id").get<string>();
        }catch(json::exception){
            throw std::runtime_error("Project ID cannot be null for pubsub connector\n");
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

        auth_options_ = gcloud::Options{}.set<gcloud::UnifiedCredentialsOption>(
            gcloud::MakeServiceAccountCredentials(get_key_contents(key_path_)));

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

        auto sub_admin = gcloud::pubsub_admin::SubscriptionAdminClient(
                    gcloud::pubsub_admin::MakeSubscriptionAdminConnection(auth_options_));
        if(mode_ == ConnectorMode::IN){
            // We always have subscription_id here, either from config or created in the
            // constructor
            auto subscription = pubsub::Subscription(project_id_, subscription_id_);
            auto resp = sub_admin.GetSubscription(subscription.FullName());
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
                auto create_sub_resp = sub_admin.CreateSubscription(
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
                        gcloud::MakeServiceAccountCredentials(get_key_contents(key_path_)))
            ));
        }else if(mode_ == ConnectorMode::OUT){
            create_topic();
            publisher_ptr_ = new pubsub::Publisher(pubsub::MakePublisherConnection(
                pubsub::Topic(project_id_, topic_id_),
                ::google::cloud::Options{}
                    .set<pubsub::MaxConcurrencyOption>(1)
                    .set<::google::cloud::GrpcBackgroundThreadPoolSizeOption>(2)
                    .set<gcloud::UnifiedCredentialsOption>(
                        gcloud::MakeServiceAccountCredentials(get_key_contents(key_path_)))
            ));
        }
    }

    MessageWrapper* receive()override{
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
            throw std::runtime_error("Error pulling messages from gcp pubsub");
        }
    }

    void send(MessageWrapper & msg_w)override{
        try{
            auto mb = pubsub::MessageBuilder{}.SetData(msg_w.msg.get_text());
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
            std::cout<<" published message "<<msg_w.msg.get_text()<<" with id= "<<*id<<std::endl;
        }
        catch (google::cloud::Status const& status) {
            std::cerr<<"google::cloud::Status thrown: "<< status<<std::endl;
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
                string vvalue = msg_w.get_topic_level(topic_level);
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
        auto topic_admin = gcloud::pubsub_admin::TopicAdminClient(
            gcloud::pubsub_admin::MakeTopicAdminConnection(auth_options_)
        );
        std::string topic_url = "projects/" + project_id_ + "/topics/" + topic_id_;
        auto res = topic_admin.CreateTopic(topic_url);
        // Throw error if topic doesn't already exists
        if (! res && res.status().code() != gcloud::StatusCode::kAlreadyExists){
            std::cerr << "Error creating topic: "<<res.status()<<std::endl;
            throw std::runtime_error("Error creating topic");
        }
    }

    std::string get_key_contents(std::string const& keyfile){
        auto is = std::ifstream(keyfile);
        is.exceptions(std::ios::badbit);
        auto contents = std::string(std::istreambuf_iterator<char>(is.rdbuf()), {});
        return contents;

    }
};

};


#endif