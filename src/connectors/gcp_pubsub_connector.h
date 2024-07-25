#include <string>
#include <map>

#include "google/cloud/pubsub/publisher.h"

#include "connector.h"


namespace gcp {

namespace pubsub = ::google::cloud::pubsub;

using std::string;
using std::map;


class PubSubConnector: public Connector {
public:
    PubSubConnector(const string &project_id): project_id_(project_id) {
    }

    void start() {
    }

    void send(const Message &msg, const Route &route) {
        string topic = route.getTopic();

        Publisher *publisher = nullptr;
        auto found = publishers_.find(topic);
        if (found == publishers_.end()) {
            publisher = new pubsub::Publisher(
                pubsub::MakePublisherConnection(pubsub::Topic(project_id_, topic)));
        } else {
            publisher = found->second;
        }

        Message gcp_message =  pubsub::MessageBuilder{}.SetData("Hello World!").Build();
        auto id = publisher.Publish(gcp_message).get();
        if (!id) {
            throw std::move(id).status();
        }
    }

private:
    string project_id_;
    map<string, pubsub::Publisher*> publishers_;
};

};