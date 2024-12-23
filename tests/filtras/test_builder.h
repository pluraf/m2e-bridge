#ifndef TEST_BUILDER_FILTRA_H
#define TEST_BUILDER_FILTRA_H

#include <catch2/catch_all.hpp>

#include  "../src/filtras/builder.h"
#include "mock_pipeline.h"


TEST_CASE("BuilderFT", "[builder_filtra]"){
    MockPipeline mock_pi;

    json filtras = {
        {"name", "cooler_on"},
        {"type", "builder"},
        {"msg_format", "json"},
        {"payload", {{"cooler_on", true}}},
        {"goto_passed", "out"}
    };

    json payload = filtras.at("payload");

    std::string key;
    bool value;
    for(auto it = payload.begin(); it != payload.end(); ++it){
        key = it.key();
        value = it.value();
    }

    json initial_msg_j = {{key, false}};
    Message msg_j(initial_msg_j.dump(), "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w_j(msg_j);

    std::string initial_msg = "key: false";
    Message msg(initial_msg, "/topc/test");
    MessageWrapper msg_w(msg);

    SECTION("Msg format is json"){
        BuilderFT builder_ft(mock_pi, filtras);

        builder_ft.process_message(msg_w_j);
        REQUIRE(msg_w_j.msg().get_json() == payload);
        REQUIRE(msg_w_j.is_passed());
    }

    filtras.at("msg_format") = "raw";
    SECTION("Msg format is not json"){
        BuilderFT builder_ft(mock_pi, filtras);
        REQUIRE_THROWS_AS(builder_ft.process_message(msg_w), std::runtime_error);
    }
}

#endif