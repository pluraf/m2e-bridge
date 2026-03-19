#ifndef TEST_LIMITER_FILTRA_H
#define TEST_LIMITER_FILTRA_H

#include <catch2/catch_all.hpp>

#include  "../src/filtras/limiter.h"
#include "mock_pipeline.h"


TEST_CASE("LimiterFT", "[limiter_filtra]"){
    MockPipeline mock_pi;

    json filtras = {
        {"type", "limiter"},
        {"encoder", "json"},
        {"decoder", "json"},
        {"size", 9999},
        {"goto_passed", "out"}
    };

    unsigned long size = filtras.at("size");

    json initial_msg = {"value1", 2, 3, "value5"};
    MessageWrapper msg_w(
        std::make_shared<Message>(initial_msg.dump(), MessageFormat::Type::JSON, "/topc/test")
    );

    string const & msg_data = msg_w.msg().get_raw();

    SECTION("Data size is smaller then size limit"){
        LimiterFT limiter_ft(mock_pi, filtras);
        limiter_ft.process_message(msg_w);

        REQUIRE(msg_w.is_passed());
    }

    filtras.at("size") = msg_data.size();
    size = msg_data.size();
    SECTION("Data size is equal to size limit"){
        LimiterFT limiter_ft(mock_pi, filtras);
        limiter_ft.process_message(msg_w);

        REQUIRE(msg_w.is_passed());
    }

    filtras.at("size") = 1;
    size = 1;
    SECTION("Data size is bigger then size limit"){
        LimiterFT limiter_ft(mock_pi, filtras);
        limiter_ft.process_message(msg_w);

        REQUIRE_FALSE(msg_w.is_passed());
    }
}

#endif