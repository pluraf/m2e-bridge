#ifndef TEST_NOP_FILTRA_H
#define TEST_NOP_FILTRA_H

#include <catch2/catch_all.hpp>

#include  "../src/filtras/nop.h"
#include "mock_pipeline.h"


TEST_CASE("NopFT", "[nop_filtra]"){
    MockPipeline mock_pi;

    nlohmann::json filtras = {
        {"type", "nop"},
        {"encoder", "json"},
        {"decoder", "json"},
        {"goto_passed", "out"}
    };

    nlohmann::json initial_msg = {{"value1", "2", "3", "value4"}};
    Message msg(initial_msg.dump(), "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w(msg);

    NopFT nop_ft(mock_pi, filtras);

    nop_ft.process_message(msg_w);

    REQUIRE(msg_w.is_passed());
}

#endif