#ifndef TEST_FILTRA_FACTORY_H
#define TEST_FILTRA_FACTORY_H

#include <catch2/catch_all.hpp>

#include  "../src/factories/filtra_factory.h"
#include "../filtras/mock_pipeline.h"


TEST_CASE("FiltraF", "[filtra_factory]"){
    MockPipeline mock_pi;

    std::string type = GENERATE("builder", "comparator", "eraser", "finder", "limiter", "nop", "splitter", "throttle", "unknown");

    nlohmann::json filtras = {
        {"name", "cooler_on"},
        {"type", type},
        {"msg_format", "json"},
        {"operator", "eq"},
        {"value_key", "temp"},
        {"keys", {"temp", "hum", "moi"}},
        {"comparand", 45},
        {"size", 9999},
        {"chunk_size", 5},
        {"rate", 1},
        {"payload", {{"cooler_on", true}}},
        {"goto_passed", "out"},
        {"goto_rejected", "cooler_off"},
    };


    if(type == "unknown"){
        REQUIRE_THROWS_AS(FiltraFactory::create(mock_pi, filtras), std::invalid_argument);
    }else{
        Filtra *filtra = FiltraFactory::create(mock_pi, filtras);
        REQUIRE(filtra != nullptr);
        if(type == "builder"){
            auto *builder_ft = dynamic_cast<BuilderFT *>(filtra);
            REQUIRE(builder_ft != nullptr);
        }else if(type == "comparator"){
            auto *comparator_ft = dynamic_cast<ComparatorFT *>(filtra);
            REQUIRE(comparator_ft != nullptr);
        }else if(type == "eraser"){
            auto *eraser_ft = dynamic_cast<EraserFT *>(filtra);
            REQUIRE(eraser_ft != nullptr);
        }else if(type == "finder"){
            auto *finder_ft = dynamic_cast<FinderFT *>(filtra);
            REQUIRE(finder_ft != nullptr);
        }else if(type == "limiter"){
            auto *limiter_ft = dynamic_cast<LimiterFT *>(filtra);
            REQUIRE(limiter_ft != nullptr);
        }else if(type == "nop"){
            auto *nop_ft = dynamic_cast<NopFT *>(filtra);
            REQUIRE(nop_ft != nullptr);
        }else if(type == "splitter"){
            auto *splitter_ft = dynamic_cast<SplitterFT *>(filtra);
            REQUIRE(splitter_ft != nullptr);
        }else if(type == "throttle"){
            auto *throttle_ft = dynamic_cast<ThrottleFT *>(filtra);
            REQUIRE(throttle_ft != nullptr);
        }
        delete filtra;
    }
}

#endif