/*
Copyright Pluraf Embedded AB, 2024
code@pluraf.com
*/

#include <fstream>
#include <iostream>

#include "global_state.h"
#include "pipeline.h"
#include "factories/filter_factory.h"
#include "factories/transformer_factory.h"
#include "factories/connector_factory.h"


Pipeline::Pipeline(std::string const & pipeid, json const & pjson){
    pipeid_ = pipeid;
    connector_in_ = ConnectorFactory::create(pjson["connector_in"], ConnectorMode::IN, pipeid_);
    for (auto filter_parsed : pjson["filters"]) {
        Filter *filter = FilterFactory::create(filter_parsed);
        filters_.push_back(filter);
    }
    for (auto transformer_parsed : pjson["transformers"]) {
        Transformer *transformer = TransformerFactory::create(transformer_parsed);
        transformers_.push_back(transformer);
    }
    connector_out_ = ConnectorFactory::create(pjson["connector_out"], ConnectorMode::OUT, pipeid_);
}


void Pipeline::run() {
    connector_in_->connect();
    connector_out_-> connect();
    while(! stop_){
        MessageWrapper* msg_w = connector_in_->receive();
        std::cout<<pipeid_<<" "<<stop_<<std::endl;
        // if no message received, continue till thread is stopped
        if(msg_w == nullptr)continue;
        if(! filter(*msg_w)){
            continue;
        }
        transform(*msg_w);
        connector_out_->send(*msg_w);
    }

    connector_in_->disconnect();
    connector_out_-> disconnect();
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
    if(th_ == nullptr){
        th_ = new std::thread(&Pipeline::run, this);
    }
}


void Pipeline::stop(){
    stop_ = true;
    connector_in_->stop();
    if(th_ != nullptr){
        th_->join();
        delete th_;
        th_ = nullptr;
    }
}
