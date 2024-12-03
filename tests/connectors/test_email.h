#ifndef TEST_EMAIL_CONNECTOR_H
#define TEST_EMAIL_CONNECTOR_H

#include <catch2/catch_all.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include  "../src/connectors/email_connector.h"
#include "mock_database.h"



namespace TestEmailConnector {
    inline void setup_test_environment(){
        Database db;

        AuthBundle test_bundle_correct;
        test_bundle_correct.authbundle_id = "test_t";
        test_bundle_correct.service_type = ServiceType::EMAIL;
        test_bundle_correct.auth_type = AuthType::PASSWORD;
        test_bundle_correct.username = "mock_user";
        test_bundle_correct.password = "mock_pass";
        db.insert_authbundle(test_bundle_correct);

        AuthBundle test_bundle_wrong;
        test_bundle_wrong.authbundle_id = "test_f";
        test_bundle_wrong.service_type = ServiceType::EMAIL;
        test_bundle_wrong.auth_type = AuthType::PASSWORD;
        test_bundle_wrong.username = "test_user";
        test_bundle_wrong.password = "test_pass";
        db.insert_authbundle(test_bundle_wrong);
    }

    inline void cleanup_test_environment(){
        Database db;

        db.delete_authbundle("test_t");
        db.delete_authbundle("test_f");
    }
}


class EmailConnectorTests {
public:
    static void setup(){
        create_test_database("../configs/test_config.json");
        TestEmailConnector::setup_test_environment();
    }

    static void run_tests_with_correct_credentials(){
        nlohmann::json config = {
            {"type", "email"},
            {"to", "mock_recipient@smtp.com"},
            {"authbundle_id", "test_t"},
            {"smtp_server", "smtp.gmail.com"}
        };

        EmailConnector email_connector("test_email_pipeline", ConnectorMode::OUT, config);

        SECTION("Parse authbundle"){
            REQUIRE(email_connector.authbundle_id_ == "test_t");
            REQUIRE(email_connector.username_ == "mock_user");
            REQUIRE(email_connector.password_ == "mock_pass");
        }

        SECTION("Parse values from json file"){
            REQUIRE_FALSE(email_connector.to_.empty());
            REQUIRE_FALSE(email_connector.authbundle_id_.empty());
            REQUIRE_FALSE(email_connector.smtp_server_.empty());
        }

        SECTION("Test connection - correct credentials"){
            std::ostringstream error_stream;
            std::streambuf* original_cerr = std::cerr.rdbuf(error_stream.rdbuf());

            REQUIRE_NOTHROW(email_connector.connect());

            std::cerr.rdbuf(original_cerr);
            REQUIRE(error_stream.str().empty());
        }

        SECTION("Test send - successfull"){
            email_connector.connect();

            std::string raw_data = "This is a test message";
            Message message(raw_data, "test");
            MessageWrapper msg_w(message);

            std::ostringstream error_stream;
            std::streambuf* original_cerr = std::cerr.rdbuf(error_stream.rdbuf());

            REQUIRE(msg_w);
            REQUIRE(msg_w.msg().get_raw() == raw_data);

            REQUIRE_NOTHROW(email_connector.do_send(msg_w));

            std::cerr.rdbuf(original_cerr);
            REQUIRE(error_stream.str().empty());
        }

        SECTION("Test disconnect"){
            email_connector.connect();

            REQUIRE_NOTHROW(email_connector.disconnect());
        }
    }

    static void run_tests_with_wrong_credentials(){
        nlohmann::json config = {
            {"type", "email"},
            {"to", "recipient@smtp.com"},
            {"authbundle_id", "test_f"},
            {"smtp_server", "smtp.test.com"}
        };

        EmailConnector email_connector("test_email_pipeline", ConnectorMode::OUT, config);

        SECTION("Parse authbundle"){
            REQUIRE(email_connector.authbundle_id_ == "test_f");
            REQUIRE(email_connector.username_ == "test_user");
            REQUIRE(email_connector.password_ == "test_pass");
        }

        SECTION("Parse values from json file"){
            REQUIRE_FALSE(email_connector.to_.empty());
            REQUIRE_FALSE(email_connector.authbundle_id_.empty());
            REQUIRE_FALSE(email_connector.smtp_server_.empty());
        }

        SECTION("Test connection - wrong credentials"){
            std::ostringstream error_stream;
            std::streambuf* original_cerr = std::cerr.rdbuf(error_stream.rdbuf());

            REQUIRE_NOTHROW(email_connector.connect());

            std::cerr.rdbuf(original_cerr);
            REQUIRE_FALSE(error_stream.str().empty());
            REQUIRE(error_stream.str().find("Connection failed") != std::string::npos);
        }

        SECTION("Test send - failed"){
            email_connector.connect();

            std::string raw_data = "This is a test message";
            Message message(raw_data, "test");
            MessageWrapper msg_w(message);

            std::ostringstream error_stream;
            std::streambuf* original_cerr = std::cerr.rdbuf(error_stream.rdbuf());

            REQUIRE(msg_w);
            REQUIRE(msg_w.msg().get_raw() == raw_data);

            REQUIRE_NOTHROW(email_connector.do_send(msg_w));

            std::cerr.rdbuf(original_cerr);
            REQUIRE_FALSE(error_stream.str().empty());
        }

        SECTION("Test disconnect"){
            email_connector.connect();

            REQUIRE_NOTHROW(email_connector.disconnect());
        }
    }
};


TEST_CASE("Email Connector", "[email_connector]"){
    EmailConnectorTests::setup();
    EmailConnectorTests::run_tests_with_correct_credentials();
    EmailConnectorTests::run_tests_with_wrong_credentials();
    TestEmailConnector::cleanup_test_environment();
}

#endif