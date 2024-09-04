#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include <csignal>

#include "pipeline.h"


#include "google/cloud/pubsub/publisher.h"
#include "google/cloud/pubsub/message.h"
#include "google/cloud/pubsub/subscriber.h"
#include "m2e_message/message_wrapper.h"

std::atomic<bool> running(true);

void signalHandler(int signal) {
    std::cout << "\nCaught signal " << signal << ". Ending call..." << std::endl;
    running = false;
}

int main(int argc, char* argv[]) {


     // Set up signal handler for Ctrl+C (SIGINT)
    std::signal(SIGINT, signalHandler);

    // namespace pubsub = ::google::cloud::pubsub;


    // std::string project_id_ = "feisty-enigma-416319";
    // std::string topic_id_ = "m2e-topic1";
    // std::string subscription_id_ = "m2e-topic1-sub";

    // Message msg = Message("test msg4", "test topic");
    // MessageWrapper mwrap = MessageWrapper(msg);


    // try {

    //     pubsub::Publisher publisher =  pubsub::Publisher(
    //                 pubsub::MakePublisherConnection(pubsub::Topic(project_id_, topic_id_))
    //             );
    //     auto id =
    //     publisher
    //         .Publish(pubsub::MessageBuilder{}.SetData(mwrap.msg.get_msg_text()).Build())
    //         .get();
    //     if (!id) throw std::move(id).status();
    //     std::cout << "Hello World published with id=" << *id << "\n";
    // }
    // catch (google::cloud::Status const& status) {
    //     std::cerr << "google::cloud::Status thrown: " << status << "\n";
    //     return 1;

    // }
    // try{

    //      auto constexpr kWaitTimeout = std::chrono::seconds(30);


    //      pubsub::Subscriber subscriber = pubsub::Subscriber(pubsub::MakeSubscriberConnection(
    //             pubsub::Subscription(project_id_, subscription_id_))
    //         );

    //     auto session =
    //   subscriber.Subscribe([&](pubsub::Message const& m, pubsub::AckHandler h) {
    //     std::cout << "Received message " << m << "\n";
    //     std::move(h).ack();
    //   });

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
    std::string pipeline_json_path = "/pipeline_test.json";

    if(argc == 2){
        pipeline_json_path = std::string(argv[1]);
    }

    std::string file_path = PROJECT_SOURCE_DIR + pipeline_json_path;

    std::ifstream file(file_path);
    if (!file) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();
    std::string json_str = buffer.str();

    long ctr = 1;

    Pipeline myPipeline(json_str);
    myPipeline.start();
    std::cout<<"started pipeline"<<std::endl;
    while(running){
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }
    myPipeline.stop();
    std::cout << "end of pipeline"<<std::endl;
    return 0;
}