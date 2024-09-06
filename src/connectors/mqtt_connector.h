#ifndef __M2E_BRIDGE_MQTT_CONNECTOR_H__
#define __M2E_BRIDGE_MQTT_CONNECTOR_H__

#include <string>
#include <atomic>
#include <stdexcept>
#include <random>

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
            std::cout << "Connection attempt failed " <<connector_ptr_->connector_type_<< std::endl;
            if (++nretry_ > connector_ptr_->n_retry_attempts_)
                exit(1);
            reconnect();
        }

        // (Re)connection success
        // Either this or connected() can be used for callbacks.
        void on_success(const mqtt::token& tok) override {}

        // (Re)connection success
        void connected(const std::string& cause) override {

            std::cout << "\nConnection success "  <<connector_ptr_->connector_type_<< std::endl;

            if( connector_ptr_->connector_type_ == "connector_in" ){
                std::cout << "\nSubscribing to topic '" << connector_ptr_->topic_ << "'\n"
                    << " using QoS" << connector_ptr_->qos_ << std::endl;

                try {
                    connector_ptr_->client_ptr_->subscribe(connector_ptr_->topic_, connector_ptr_->qos_, nullptr, subListener_);
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
            connector_ptr_->msg_queue_->put(msg->to_string());
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
    string topic_;
    int n_retry_attempts_;
    int qos_;
    mqtt::connect_options conn_opts_;
    mqtt::thread_queue<string> *msg_queue_;
    Callback* callback_ptr_;


public:
    MqttConnector(nlohmann::json json_descr, std::string type):
     Connector(json_descr, type) {
        if(json_descr["server"].is_null()){
            throw std::runtime_error("Server url cannot be null for mqtt connector\n");
        }
        if(json_descr["topic"].is_null()){
            throw std::runtime_error("Topic cannot be null for mqtt connector\n");
        }
        server_ = json_descr["server"];
        topic_ = json_descr["topic"];
        client_id_ = json_descr["client_id"].is_null() ?  generate_random_id(10) : json_descr["client_id"].get<std::string>();;
        n_retry_attempts_ = json_descr["n_retry_attempts"].is_null() ?  N_RETRY_ATTEMPTS: json_descr["n_retry_attempts"].get<int>();
        qos_ = json_descr["qos"].is_null() ?  QOS : json_descr["qos"].get<int>();

        std::cout<<"server "<<server_<<std::endl;

        std::cout<<"client id  "<<client_id_<<std::endl;
        msg_queue_ = new mqtt::thread_queue<string>(1000);
        client_ptr_ = std::make_shared<mqtt::async_client>(server_, client_id_);
    }

    void connect() override {
        conn_opts_.set_clean_session(false);

        // Install the callback(s) before connecting.
        callback_ptr_ = new Callback(this);
        client_ptr_->set_callback(*callback_ptr_);
        try {
            std::cout << "Connecting to the MQTT server... for " <<connector_type_<< std::endl;
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

    void send(const MessageWrapper &msg_w) override {
        try {
            std::cout << "\nSending next message..." << std::endl;
            mqtt::delivery_token_ptr pubtok;
            pubtok = client_ptr_->publish(
                topic_,
                msg_w.msg.get_msg_text().c_str(),
                msg_w.msg.get_msg_text().length(),
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
        string msg_text;
        msg_queue_->get(&msg_text);  // blocking call
        Message msg(msg_text, topic_);
        return new MessageWrapper(msg);
    }



};

#endif