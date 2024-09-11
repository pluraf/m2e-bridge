#ifndef __M2E_BRIDGE_GCP_PUBSUB_CONNECTOR_H__
#define __M2E_BRIDGE_GCP_PUBSUB_CONNECTOR_H__


#include <string>
#include <map>
#include <iostream>
#include <utility>
#include <stdexcept>
#include <cstdlib>
#include <regex>

#include "google/cloud/pubsub/publisher.h"
#include "google/cloud/pubsub/subscriber.h"
#include "google/cloud/pubsub/message.h"
#include "google/cloud/pubsub/admin/topic_admin_client.h"
#include "google/cloud/pubsub/topic.h"
#include "google/cloud/status.h"

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

    const char* KEY_ENV_VARIABLE = "GOOGLE_APPLICATION_CREDENTIALS";
    pubsub::Publisher* publisher_ptr_;
    pubsub::Subscriber* subscriber_ptr_;

    map<string, std::pair<bool,string>> attributes_;

public:
    PubSubConnector(json const & json_descr, ConnectorMode mode):Connector(json_descr, mode){
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
        if(mode_ == ConnectorMode::IN){
            subscriber_ptr_ = new pubsub::Subscriber(pubsub::MakeSubscriberConnection(
                pubsub::Subscription(project_id_, subscription_id_))
            );
        }else if(mode_ == ConnectorMode::OUT){
            auto topic_admin_client = gcloud::pubsub_admin::TopicAdminClient(
                gcloud::pubsub_admin::MakeTopicAdminConnection());
            std::string topic_url = "projects/"+project_id_+"/topics/"+topic_id_;
            auto topic_creation_response = topic_admin_client.CreateTopic(topic_url);
            // Throw error if topic doesn't already exists
            if (!topic_creation_response && 
                topic_creation_response.status().code() != gcloud::StatusCode::kAlreadyExists
            ) {
                std::cerr << "Error creating topic: " << topic_creation_response.status() << "\n";
                throw std::runtime_error("Error creating topic");
                
            }
            publisher_ptr_ = new pubsub::Publisher(
                pubsub::MakePublisherConnection(pubsub::Topic(project_id_, topic_id_))
            );
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
};

};


#endif