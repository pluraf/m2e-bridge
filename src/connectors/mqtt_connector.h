#ifndef __M2E_BRIDGE_MQTT_CONNECTOR_H__
#define __M2E_BRIDGE_MQTT_CONNECTOR_H__

#include <string>
#include <atomic>
#include <stdexcept>
#include <random>
#include <regex>
#include <format>

#include "mqtt/async_client.h"

#include "connector.h"

const int QOS=1;
const int	N_RETRY_ATTEMPTS = 5;

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
        std::cout << name_ << " success";
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


class MqttConnector: virtual public Connector {
private:
    class Callback : public virtual mqtt::callback,
                    public virtual mqtt::iaction_listener

    {
        // Counter for the number of connection retries
        int nretry_;
        MqttConnector* connector_ptr_;
        ActionListener subListener_;

        void reconnect() {
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            try {
                connector_ptr_->client_ptr_->connect(connector_ptr_->conn_opts_, nullptr, *this);
            }
            catch (const mqtt::exception& exc) {
                std::cerr << "Error: " << exc.what() << std::endl;
                exit(1);
            }
        }

        // Re-connection failure
        void on_failure(const mqtt::token& tok) override {
            std::cout << "Connection attempt failed" << std::endl;
            if (++nretry_ > connector_ptr_->n_retry_attempts_)
                exit(1);
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
                    throw std::runtime_error("Unable to subscribe\n");
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
            std::cout << "Message arrived" << std::endl;
            std::cout << "\ttopic: '" << msg->get_topic() << "'" << std::endl;
            std::cout << "\tpayload: '" << msg->to_string() << "'\n" << std::endl;
            connector_ptr_->msg_queue_->put(*msg);
        }

        void delivery_complete(mqtt::delivery_token_ptr token) override {}

    public:
        Callback(MqttConnector *connector):
        nretry_(0),
        connector_ptr_(connector),
        subListener_("Subscription")
        {}
    };

    string server_;
    string client_id_;
    mqtt::async_client_ptr  client_ptr_;
    string topic_template_;
    bool is_topic_template_ {false};
    int n_retry_attempts_;
    int qos_;
    mqtt::connect_options conn_opts_;
    mqtt::thread_queue<mqtt::message> * msg_queue_;
    Callback* callback_ptr_;


public:
    MqttConnector(
            json const & json_descr, ConnectorMode mode, std::string pipeid
        ):Connector(json_descr, mode, pipeid){
        try{
            server_ = json_descr.at("server").get<string>();
        }catch(json::exception){
            throw std::runtime_error("Server url cannot be null for mqtt connector\n");
        }
        try{
            topic_template_ = json_descr.at("topic").get<string>();
        }catch(json::exception){
           throw std::runtime_error("Topic cannot be null for mqtt connector\n");
        }
        try{
            client_id_ = json_descr.at("client_id").get<string>();
        }catch(json::exception){
            client_id_ = generate_random_id(10);
        }
        try{
            n_retry_attempts_ = json_descr.at("n_retry_attempts").get<int>();
        }catch(json::exception){
            n_retry_attempts_ = N_RETRY_ATTEMPTS;
        }
        try{
            qos_ = json_descr.at("qos").get<int>();
        }catch(json::exception){
            qos_ = QOS;
        }

        std::cout<<"server "<<server_<<std::endl;

        std::cout<<"client id  "<<client_id_<<std::endl;
        msg_queue_ = new mqtt::thread_queue<mqtt::message>(1000);
        client_ptr_ = std::make_shared<mqtt::async_client>(server_, client_id_);

        std::smatch match;
        std::regex pattern("\\{\\{(.*?)\\}\\}");
        is_topic_template_ = std::regex_search(topic_template_.cbegin(), topic_template_.cend(), match, pattern);
    }

    void connect() override {
        conn_opts_.set_clean_session(false);

        // Install the callback(s) before connecting.
        callback_ptr_ = new Callback(this);
        client_ptr_->set_callback(*callback_ptr_);
        try {
            std::cout << "Connecting to the MQTT server..." << std::endl;
            client_ptr_->connect(conn_opts_, nullptr, *callback_ptr_)->wait();
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "\nERROR: Unable to connect to MQTT server: '"
                << server_ << "'" << exc << std::endl;
            throw std::runtime_error("Unable to connect to MQTT server\n");
        }
    }
    void disconnect() override {
        try {
            std::cout << "\nDisconnecting from the MQTT server..." << std::flush;
            client_ptr_->disconnect()->wait();
            std::cout << "OK" << std::endl;
        }
        catch (const mqtt::exception& exc) {
            std::cerr << exc << std::endl;
            throw std::runtime_error("Unable to disconnect from MQTT server\n");
        }
    }

    void send(MessageWrapper & msg_w)override{
        using namespace std;

        string derived_topic;
        if(is_topic_template_){
            try{
                derived_topic = derive_topic(msg_w);
            }catch(runtime_error const & e){
                cerr<<e.what()<<endl;
                return;
            }
        }

        string const & topic = is_topic_template_ ? derived_topic : topic_template_;
        try {
            mqtt::delivery_token_ptr pubtok;
            cout<<"\nSending next message... topic: "<<topic<<std::endl;
            pubtok = client_ptr_->publish(
                topic,
                msg_w.msg.get_text().c_str(),
                msg_w.msg.get_text().length(),
                qos_,
                false);
            std::cout << "  ...with token: " << pubtok->get_message_id() << std::endl;
            std::cout << "  ...for message with " << pubtok->get_message()->get_payload().size()
                << " bytes" << std::endl;
            pubtok->wait_for(TIMEOUT);
            std::cout << "  ...OK" << std::endl;
        }
        catch (const mqtt::exception& exc) {
            std::cerr << exc << std::endl;
            throw std::runtime_error("Unable to send message to MQTT server\n");
        }
    }

    MessageWrapper * receive() override {
        mqtt::message mqtt_msg;
        try{
            msg_queue_->get(&mqtt_msg);  // blocking call
        }catch(const std::underflow_error){
            return nullptr;
        }
        Message msg(mqtt_msg.to_string(), mqtt_msg.get_topic());
        return new MessageWrapper(msg);
    }

    void stop()override{
        msg_queue_->handle_exit();
    }

    std::string derive_topic(MessageWrapper & msg_w){
        using namespace std;

        regex pattern("\\{\\{(.*?)\\}\\}");
        smatch match;

        string topic = topic_template_;
        try{
            json const & payload = msg_w.get_payload();
            auto pos = topic.cbegin();
            while(regex_search(pos, topic.cend(), match, pattern)){
                string vname = match[1].str();
                try{
                    string vvalue = payload.at(vname);
                    topic.replace(match.position(), match.length(), vvalue);
                    pos = pos + match.position() + vvalue.size();
                }catch(json::exception){
                    throw runtime_error(format("Topic template variable {} not found!", vname));
                }
            }
        }catch(json::exception){
            throw runtime_error("Message payload is not a valid JSON!");
        }
        return topic;
    }
};

#endif