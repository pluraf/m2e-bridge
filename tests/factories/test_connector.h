#ifndef TEST_CONNECTOR_FACTORY_H
#define TEST_CONNECTOR_FACTORY_H

#include <catch2/catch_all.hpp>

#include  "../src/factories/connector_factory.h"

#include "../../src/internal_queues.cpp"
#include "../connectors/mock_database.h"



namespace TestConnectorFactory {
    inline void setup_test_environment(){
        Database db;

        AuthBundle test_bundle_email;

        test_bundle_email.authbundle_id = "test_email";
        test_bundle_email.service_type = ServiceType::EMAIL;
        test_bundle_email.auth_type = AuthType::PASSWORD;
        test_bundle_email.username = "test_user";
        test_bundle_email.password = "test_pass";

        db.insert_authbundle(test_bundle_email);

        AuthBundle test_bundle_s3;

        test_bundle_s3.authbundle_id = "test_s3";
        test_bundle_s3.service_type = ServiceType::AWS;
        test_bundle_s3.auth_type = AuthType::ACCESS_KEY;
        test_bundle_s3.username = "test_user";
        test_bundle_s3.password = "test_acckey";

        db.insert_authbundle(test_bundle_s3);

        AuthBundle test_bundle_bucket;

        test_bundle_bucket.authbundle_id = "test_bucket";
        test_bundle_bucket.service_type = ServiceType::GCP;
        test_bundle_bucket.auth_type = AuthType::SERVICE_KEY;
        test_bundle_bucket.keydata = "test_keydata";

        db.insert_authbundle(test_bundle_bucket);
    }

    inline void cleanup_test_environment(){
        Database db;

        db.delete_authbundle("test_email");
        db.delete_authbundle("test_s3");
        db.delete_authbundle("test_bucket");
    }
}


TEST_CASE("ConnectorF", "[connector_factory]"){
    ConnectorMode mode = GENERATE(ConnectorMode::IN, ConnectorMode::OUT);

    auto [type, config] = GENERATE(
        std::make_tuple("mqtt", nlohmann::json{
            {"type", "mqtt"},
            {"topic", "/topic2"},
            {"server", "mqtt://localhost:1883"},
            {"client_id", "xyz"},
            {"qos", 1},
            {"retry_attempts", 5},
            {"authbundle_id", "e3ad4683b2394c5498321c85e71f1d70"}
        }), 
        std::make_tuple("gcp_pubsub", nlohmann::json{
            {"type", "gcp_pubsub"},
            {"authbundle_id", "95b0bfe15c05408c91ce6933ea85d904"},
            {"project_id", "feisty-enigma-416319"},
            {"topic_id", "mock-m2e-topic"},
            {"subscription_id", "m2e-topic1-sub"}
        }),
        std::make_tuple("queue", nlohmann::json{
            {"type", "queue"},
            {"name", "mock-queue"}
        }),
        std::make_tuple("gcp_bucket", nlohmann::json{
            {"type", "gcp_bucket"},
            {"authbundle_id", "test_bucket"},
            {"project_id", "feisty-enigma-416319"},
            {"bucket_name", "m2e-bridge-bucket"},
            {"object_name", "mock_bucket"}
        }),
        std::make_tuple("aws_s3", nlohmann::json{
            {"type", "aws_s3"},
            {"bucket_name", "test-bucket"},
            {"object_name", "{{topic}}_{{timestamp}}"},
            {"authbundle_id", "test_s3"},
            {"delete_received", true}
        }),
        std::make_tuple("unknown", nlohmann::json{
            {"type", "unknown"}
        })
    );

    create_test_database("../configs/test_config.json");
    TestConnectorFactory::setup_test_environment();

    if(type == "unknown"){
        REQUIRE_THROWS_AS(ConnectorFactory::create("mock_pi", mode, config), std::invalid_argument);
    }else{
        Connector *connector = ConnectorFactory::create("mock_pi", mode, config);
        if(type == "mqtt"){
            SECTION("mqtt"){
                auto *mqtt_connector = dynamic_cast<MqttConnector *>(connector);
                REQUIRE(mqtt_connector != nullptr);
            }
        }else if(type == "gcp_pubsub"){
            SECTION("gcp_pubsub"){
                auto *pubsub_connector = dynamic_cast<gcp::PubSubConnector *>(connector);
                REQUIRE(pubsub_connector != nullptr);
            }
        }else if(type == "queue"){
            SECTION("queue"){
                auto *internal_connector = dynamic_cast<InternalConnector *>(connector);
                REQUIRE(internal_connector != nullptr);
            }
        }else if(type == "gcp_bucket"){
            SECTION("gcp_bucket"){
                auto *bucket_connector = dynamic_cast<CloudStorageConnector *>(connector);
                REQUIRE(bucket_connector != nullptr);
            }
        }else if(type == "aws_s3"){
            SECTION("aws_s3"){
                auto *s3_connector = dynamic_cast<S3Connector *>(connector);
                REQUIRE(s3_connector != nullptr);
            }
        }
    }


    SECTION("email"){
        nlohmann::json email_config = {{"type", "email"},
                                       {"smtp_server", "smtp.test.com"},
                                       {"to", "mock@pluraf.com"},
                                       {"authbundle_id", "test_email"}
                                      };

        if(mode == ConnectorMode::OUT){
            Connector *connector = ConnectorFactory::create("mock_pi", mode, email_config);
            auto *email_connector = dynamic_cast<EmailConnector *>(connector);
            REQUIRE(email_connector != nullptr);
        }else{
            REQUIRE_THROWS_AS(ConnectorFactory::create("mock_pi", mode, email_config), std::runtime_error);
        }
    }

    TestConnectorFactory::cleanup_test_environment();
}

#endif