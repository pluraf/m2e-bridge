/*
Copyright Pluraf Embedded AB, 2024
code@pluraf.com
*/

#include <fstream>

#include <nlohmann/json.hpp>

#include "pipeline.h"
#include "factories/filter_factory.h"
#include "factories/transformer_factory.h"
#include "factories/connector_factory.h"


Pipeline::Pipeline(const std::string &json_str) {
    using json = nlohmann::json;

    json parsed = json::parse(json_str);
    connector_in_ = ConnectorFactory::create(parsed["connector_in"]);
    for (auto filter_parsed : parsed["filters"]) {
        Filter *filter = FilterFactory::create(filter_parsed);
        filters_.push_back(filter);
    }
    for (auto transformer_parsed : parsed["transformers"]) {
        Transformer *transformer = TransformerFactory::create(transformer_parsed);
        transformers_.push_back(transformer);
    }
    connector_out_ = ConnectorFactory::create(parsed["connector_out"]);
}


void Pipeline::run() {
    connector_in_->connect();

    while (!stop_) {
        MessageWrapper msg_w = connector_in_->receive();
        if (! filter(msg_w)) {
            continue;
        }
        transform(msg_w);
        connector_out_->send(msg_w);
    }

    connector_in_->disconnect();
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
