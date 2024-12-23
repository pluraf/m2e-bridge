#ifndef TEST_FINDER_FILTRA_H
#define TEST_FINDER_FILTRA_H

#include <catch2/catch_all.hpp>

#include  "../src/filtras/finder.h"
#include "mock_pipeline.h"


TEST_CASE("FinderFT - String", "[finder_filtra]"){
    MockPipeline mock_pi;

    json filtras = {
        {"type", "finder"},
        {"operator", "contain"},
        {"logical_negation", false},
        {"string", "LOG MSG"}
    };

    std::string oper = filtras.at("operator");

    bool logical_negation = filtras.at("logical_negation");

    std::string string = filtras.at("string");

    std::string initial_msg_1 = "LOG MSG";
    Message msg_1(initial_msg_1, "/topc/test");
    MessageWrapper msg_w_1(msg_1);

    std::string initial_msg_2 = "MOCK";
    Message msg_2(initial_msg_2, "/topc/test");
    MessageWrapper msg_w_2(msg_2);

    std::string initial_msg_3 = "LOG";
    Message msg_3(initial_msg_3, "/topc/test");
    MessageWrapper msg_w_3(msg_3);


    SECTION("contain - logical negation is false"){
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.msg().get_raw().find(string) != std::string::npos);
        REQUIRE(msg_w_1.is_passed());

        finder_ft.process_message(msg_w_2);
        REQUIRE_FALSE(msg_w_2.is_passed());
    }

    SECTION("contain - logical negation is true"){
        filtras.at("logical_negation") = true;
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_1);
        REQUIRE_FALSE(msg_w_1.is_passed());

        finder_ft.process_message(msg_w_2);
        REQUIRE(msg_w_2.is_passed());
    }

    filtras.at("operator") = "contained";
    SECTION("contained - logical negation is false"){
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_2);
        REQUIRE_FALSE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE(string.find(msg_w_3.msg().get_raw()) != std::string::npos);
        REQUIRE(msg_w_3.is_passed());
    }

    SECTION("contained - logical negation is true"){
        filtras.at("logical_negation") = true;
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_2);
        REQUIRE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE_FALSE(msg_w_3.is_passed());
    }

    filtras.at("operator") = "match";
    SECTION("match - logical negation is false"){
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.msg().get_raw() == string);
        REQUIRE(msg_w_1.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE_FALSE(msg_w_3.is_passed());
    }

    SECTION("match - logical negation is true"){
        filtras.at("logical_negation") = true;
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.msg().get_raw() == string);
        REQUIRE_FALSE(msg_w_1.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE(msg_w_3.is_passed());
    }
}



TEST_CASE("FinderFT - Keys", "[finder_filtra]"){
    MockPipeline mock_pi;

    json filtras = {
        {"type", "finder"},
        {"operator", "contain"},
        {"msg_format", "json"},
        {"logical_negation", false},
        {"keys", {"temp", "hum", "moi"}}
    };

    std::string oper = filtras.at("operator");

    bool logical_negation = filtras.at("logical_negation");

    json keys = filtras.at("keys");

    json initial_msg_1 = {{"temp", "value1"}, {"hum", "value2"}};
    Message msg_1(initial_msg_1, "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w_1(msg_1);

    json initial_msg_2 = {{"value1", "value1"}, {"value2", "value2"}};
    Message msg_2(initial_msg_2, "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w_2(msg_2);

    json initial_msg_3 = {{"temp", "value1"}, {"hum", "value2"}, {"moi", "value3"}};
    Message msg_3(initial_msg_3, "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w_3(msg_3);


    SECTION("contain - logical negation is false"){
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_1);
        REQUIRE_FALSE(msg_w_1.is_passed());

        finder_ft.process_message(msg_w_2);
        REQUIRE_FALSE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE(msg_w_3.is_passed());
    }

    filtras.at("logical_negation") = true;
    SECTION("contain - logical negation is true"){
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.is_passed());

        finder_ft.process_message(msg_w_2);
        REQUIRE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE_FALSE(msg_w_3.is_passed());
    }
}


TEST_CASE("FinderFT - Value Key", "[finder_filtra]"){
    MockPipeline mock_pi;

    json filtras = {
        {"type", "finder"},
        {"operator", "contain"},
        {"msg_format", "json"},
        {"logical_negation", false},
        {"value_key", "temp"},
        {"string", "correct value"}
    };

    std::string oper = filtras.at("operator");

    bool logical_negation = filtras.at("logical_negation");

    std::string val_key = filtras.at("value_key");

    json initial_msg_1 = {{"temp", "value"}, {"hum", "value"}};
    Message msg_1(initial_msg_1, "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w_1(msg_1);

    json initial_msg_2 = {{"hum", "value"}, {"moi", "value"}};
    Message msg_2(initial_msg_2, "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w_2(msg_2);

    json initial_msg_3 = {{"temp", "temp correct value"}, {"hum", "value"}};
    Message msg_3(initial_msg_3, "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w_3(msg_3);

    json initial_msg_4 = {{"temp", "correct value"}, {"moi", "value"}};
    Message msg_4(initial_msg_4, "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w_4(msg_4);


    SECTION("contain - logical negation is false"){
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_2);
        REQUIRE_FALSE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE(msg_w_3.is_passed());

        finder_ft.process_message(msg_w_1);
        REQUIRE_FALSE(msg_w_1.is_passed());
    }

    SECTION("contain - logical negation is true"){
        filtras.at("logical_negation") = true;
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_2);
        REQUIRE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE_FALSE(msg_w_3.is_passed());

        finder_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.is_passed());
    }

    filtras.at("operator") = "contained";
    SECTION("contained - logical negation is false"){
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_2);
        REQUIRE_FALSE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE_FALSE(msg_w_3.is_passed());

        finder_ft.process_message(msg_w_4);
        REQUIRE(msg_w_4.is_passed());
    }

    SECTION("contained - logical negation is true"){
        filtras.at("logical_negation") = true;
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_2);
        REQUIRE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_1);
        REQUIRE_FALSE(msg_w_1.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE(msg_w_3.is_passed());

        finder_ft.process_message(msg_w_4);
        REQUIRE_FALSE(msg_w_4.is_passed());
    }

    filtras.at("operator") = "match";
    SECTION("match - logical negation is false"){
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_2);
        REQUIRE_FALSE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_4);
        REQUIRE(msg_w_4.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE_FALSE(msg_w_3.is_passed());
    }

    SECTION("match - logical negation is true"){
        filtras.at("logical_negation") = true;
        FinderFT finder_ft(mock_pi, filtras);

        finder_ft.process_message(msg_w_2);
        REQUIRE(msg_w_2.is_passed());

        finder_ft.process_message(msg_w_4);
        REQUIRE_FALSE(msg_w_4.is_passed());

        finder_ft.process_message(msg_w_3);
        REQUIRE(msg_w_3.is_passed());
    }
}

#endif