#ifndef TEST_THROTTLE_FILTRA_H
#define TEST_THROTTLE_FILTRA_H

#include <catch2/catch_all.hpp>
#include <thread>

#include  "../src/filtras/throttle.h"
#include "mock_pipeline.h"


TEST_CASE("ThrottleFT", "[throttle_filtra]"){
    MockPipeline mock_pi;

    nlohmann::json filtras = {
        {"type", "throttle"},
        {"msg_format", "json"},
        {"rate", 1},
        {"goto_passed", "out"}
    };

    nlohmann::json initial_msg = "Hello!";
    Message msg(initial_msg, "/topc/test");

    ThrottleFT throttle_ft(mock_pi, filtras);

    SECTION("Correct rate"){
        MessageWrapper msg_w_1(msg);
        throttle_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.is_passed());

        std::this_thread::sleep_for(std::chrono::seconds(1));

        MessageWrapper msg_w_2(msg);
        throttle_ft.process_message(msg_w_2);
        REQUIRE(msg_w_2.is_passed());
    }

    SECTION("Exceeded rate"){
        MessageWrapper msg_w_1(msg);
        throttle_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.is_passed());

        MessageWrapper msg_w_2(msg);
        throttle_ft.process_message(msg_w_2);
        REQUIRE_FALSE(msg_w_2.is_passed());
    }

    SECTION("Burst of messages"){
        MessageWrapper msg_w_1(msg);
        throttle_ft.process_message(msg_w_1);
        REQUIRE(msg_w_1.is_passed());

        for(int i = 0; i < 5; ++i){
            MessageWrapper msg_w_burst(msg);
            throttle_ft.process_message(msg_w_burst);
            REQUIRE_FALSE(msg_w_burst.is_passed());
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        MessageWrapper msg_w_2(msg);
        throttle_ft.process_message(msg_w_2);
        REQUIRE(msg_w_2.is_passed());
    }
}

#endif