#ifndef TEST_ERASER_FILTRA_H
#define TEST_ERASER_FILTRA_H

#include <catch2/catch_all.hpp>

#include  "../src/filtras/eraser.h"
#include "mock_pipeline.h"


TEST_CASE("EraserFT", "[eraser_filtra]"){
    MockPipeline mock_pi;

    json filtras = {
        {"type", "eraser"},
        {"msg_format", "json"},
        {"keys", {"temp", "hum", "moi"}},
        {"goto_passed", "out"}
    };

    json keys = filtras.at("keys");
    vector<string> v_keys = vector<string>(keys.begin(), keys.end());

    json initial_msg = {{"value1", "value2", "temp", "hum"}};
    Message msg(initial_msg.dump(), "/topc/test", MessageFormat::JSON);
    MessageWrapper msg_w(msg);

    EraserFT eraser_ft(mock_pi, filtras);

    eraser_ft.process_message(msg_w);

    for(int i = 0; i < v_keys.size(); i++){
        REQUIRE_FALSE(msg_w.msg().get_json().contains(v_keys[i]));
    }
}

#endif