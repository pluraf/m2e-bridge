#ifndef TEST_COMPARATOR_FILTRA_H
#define TEST_COMPARATOR_FILTRA_H

#include <catch2/catch_all.hpp>
#include <random>

#include  "../src/filtras/comparator.h"
#include "mock_pipeline.h"


int generate_random_number(std::string type){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> int_dist(1, 100);
    std::uniform_real_distribution<> float_dist(1.0, 100.0);

    int random_int = int_dist(gen);
    float random_float = float_dist(gen);

    if(type == "int"){
        return random_int;
    }else if(type == "float"){
        return random_float;
    }else{
        throw std::invalid_argument("type");
    }
}

int random_int = generate_random_number("int");
float random_float = generate_random_number("float");


TEST_CASE("ComparatorFT", "[comparator_filtra]"){
    MockPipeline mock_pi;

    json filtras = {
        {"type", "comparator"},
        {"operator", "eq"},
        {"value_key", "temp"},
        {"comparand", 45},
        {"goto_passed", "cooler_on"},
        {"goto_rejected", "cooler_off"}
    };

    int compar = filtras.at("comparand");
    std::string val_key = filtras.at("value_key");

    json initial_msg_i = {{val_key, random_int}};
    MessageWrapper msg_wi(std::make_shared<Message>(initial_msg_i, MessageFormat::Type::JSON, "/topc/test"));

    json initial_msg_f = {{val_key, random_float}};
    MessageWrapper msg_wf(std::make_shared<Message>(initial_msg_f, MessageFormat::Type::JSON, "/topc/test"));

    json const & msg_payload_i = msg_wi.msg().get_json();
    json const & msg_payload_f = msg_wf.msg().get_json();

    json const & msg_value_i = msg_payload_i.at(val_key);
    json const & msg_value_f = msg_payload_f.at(val_key);

    REQUIRE(msg_value_i.is_number_integer());
    REQUIRE(msg_value_f.is_number_float());


    SECTION("eq"){
        ComparatorFT comparator_ft(mock_pi, filtras);

        comparator_ft.process_message(msg_wi);
        if(msg_value_i == compar){
            REQUIRE(msg_wi.is_passed());
        }else{
            REQUIRE_FALSE(msg_wi.is_passed());
        }

        comparator_ft.process_message(msg_wf);
        if(msg_value_f == compar){
            REQUIRE(msg_wf.is_passed());
        }else{
            REQUIRE_FALSE(msg_wf.is_passed());
        }
    }

    filtras.at("operator") = "gt";
    SECTION("gt"){
        ComparatorFT comparator_ft(mock_pi, filtras);

        comparator_ft.process_message(msg_wi);
        if(msg_value_i > compar){
            REQUIRE(msg_wi.is_passed());
        }else{
            REQUIRE_FALSE(msg_wi.is_passed());
        }

        comparator_ft.process_message(msg_wf);
        if(msg_value_f > compar){
            REQUIRE(msg_wf.is_passed());
        }else{
            REQUIRE_FALSE(msg_wf.is_passed());
        }
    }

    filtras.at("operator") = "gte";
    SECTION("gte"){
        ComparatorFT comparator_ft(mock_pi, filtras);

        comparator_ft.process_message(msg_wi);
        if(msg_value_i >= compar){
            REQUIRE(msg_wi.is_passed());
        }else{
            REQUIRE_FALSE(msg_wi.is_passed());
        }

        comparator_ft.process_message(msg_wf);
        if(msg_value_f >= compar){
            REQUIRE(msg_wf.is_passed());
        }else{
            REQUIRE_FALSE(msg_wf.is_passed());
        }
    }

    filtras.at("operator") = "lt";
    SECTION("lt"){
        ComparatorFT comparator_ft(mock_pi, filtras);

        comparator_ft.process_message(msg_wi);
        if(msg_value_i < compar){
            REQUIRE(msg_wi.is_passed());
        }else{
            REQUIRE_FALSE(msg_wi.is_passed());
        }

        comparator_ft.process_message(msg_wf);
        if(msg_value_f < compar){
            REQUIRE(msg_wf.is_passed());
        }else{
            REQUIRE_FALSE(msg_wf.is_passed());
        }
    }

    filtras.at("operator") = "lte";
    SECTION("lte"){
        ComparatorFT comparator_ft(mock_pi, filtras);

        comparator_ft.process_message(msg_wi);
        if(msg_value_i <= compar){
            REQUIRE(msg_wi.is_passed());
        }else{
            REQUIRE_FALSE(msg_wi.is_passed());
        }

        comparator_ft.process_message(msg_wf);
        if(msg_value_f <= compar){
            REQUIRE(msg_wf.is_passed());
        }else{
            REQUIRE_FALSE(msg_wf.is_passed());
        }
    }

    SECTION("Type of key_val"){
        ComparatorFT comparator_ft(mock_pi, filtras);
        json initial_msg = {{val_key, "value"}};
        MessageWrapper msg_w(
            std::make_shared<Message>(initial_msg.dump(), MessageFormat::Type::JSON, "/topc/test")
        );
        REQUIRE_THROWS_AS(comparator_ft.process_message(msg_w), std::invalid_argument);
    }
}

#endif