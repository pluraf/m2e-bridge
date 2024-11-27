#ifndef TEST_SPLITTER_FILTRA_H
#define TEST_SPLITTER_FILTRA_H

#include <catch2/catch_all.hpp>

#include  "../src/filtras/splitter.h"
#include "mock_pipeline.h"


TEST_CASE("SplitterFT", "[splitter_filtra]"){
    MockPipeline mock_pi;

    nlohmann::json filtras = {
        {"type", "splitter"},
        {"encoder", "json"},
        {"chunk_size", 5},
        {"goto_passed", "out"}
    };

    unsigned long chunk_size = filtras.at("chunk_size");

    std::string initial_msg = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    Message msg(initial_msg, "/topc/test");
    MessageWrapper msg_w(msg);

    string const & msg_data = msg_w.msg().get_raw();

    SECTION("Smaller chunk_size"){
        SplitterFT splitter_ft(mock_pi, filtras);

        REQUIRE(splitter_ft.process_message(msg_w) == "self");

        std::vector<std::string> expected_chunks = {
            "ABCDE", "FGHIJ", "KLMNO", "PQRST", "UVWXY", "Z"
        };

        std::vector<std::string> actual_chunks;
        Message chunk_msg;
        for(int i = 0; i < expected_chunks.size(); i++){
            chunk_msg = splitter_ft.generate_message();

            REQUIRE_FALSE(chunk_msg.get_raw().empty());

            nlohmann::json chunk_json = nlohmann::json::from_cbor(chunk_msg.get_raw());
            std::vector<std::uint8_t> chunk_binary = chunk_json[chunk_size-1].get_binary();
            std::string chunk_data(chunk_binary.begin(), chunk_binary.end());

            REQUIRE(chunk_data == expected_chunks[i]);

            actual_chunks.push_back(chunk_data);
        }
    }

    filtras.at("chunk_size") = msg_data.size();
    SECTION("Equal"){
        SplitterFT splitter_ft(mock_pi, filtras);

        splitter_ft.process_message(msg_w);
        REQUIRE(msg_w.is_passed());
    }

    filtras.at("chunk_size") = msg_data.size() + 1;
    SECTION("Biger chunk_size"){
        SplitterFT splitter_ft(mock_pi, filtras);

        splitter_ft.process_message(msg_w);
        REQUIRE(msg_w.is_passed());
    }
}

#endif