/*
Copyright Pluraf Embedded AB, 2024
code@pluraf.com
*/

#include <fstream>

#include <nlohmann/json.hpp>

#include "pipeline.h"


Pipeline::Pipeline(const std::string &json_str) {
    using json = nlohmann::json;

    json parsed = json::parse(json_str);
    for (auto filter_parsed : parsed["filters"]) {
        Filter *filter = new Filter(filter_parsed);
        filters_.push_back(filter);
    }
    for (auto transformer_parsed : parsed["transformers"]) {
        Transformer *transformer = new Transformer(transformer_parsed);
        transformers_.push_back(transformer);
    }
    for (auto mapper_parsed : parsed["mappers"]) {
        Mapper *mapper = new Mapper(mapper_parsed);
        mappers_.push_back(mapper);
    }
}


void Pipeline::run() {
    connector_in_.start();

    while (!stop_) {
        Message msg = connector_in_.receive();
        MessageWrapper msg_w = MessageWrapper(msg);
        if (! filter(msg_w)) {
            continue;
        }
        transform(msg_w);
        map(msg_w);
    }

    connector_in_.stop();
}


bool Pipeline::filter(MessageWrapper& msg_w) {
    for (auto *filter : filters_) {
        if (! filter->apply(msg_w)) {
            return false;
        }
    }
    return true;
}


void Pipeline::transform(MessageWrapper& msg_w) {
    for (auto *transformer : transformers_) {
        transformer->apply(msg_w);
    }
}


void Pipeline::map(MessageWrapper& msg_w) {
    for (auto *mapper : mappers_) {
        mapper->apply(msg_w);
    }
}


void Pipeline::start() {
    if (th_ == nullptr) {
        th_ = new std::thread(&Pipeline::run, this);
    }
}


void Pipeline::stop() {
    if (th_ != nullptr) {
        stop_ = true;
        th_->join();
        delete th_;
        th_ = nullptr;
    }
}
