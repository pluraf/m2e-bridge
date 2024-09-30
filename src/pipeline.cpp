/*
Copyright Pluraf Embedded AB, 2024
code@pluraf.com
*/

#include <fstream>
#include <iostream>
#include <stdexcept>

#include "global_state.h"
#include "pipeline.h"
#include "factories/filter_factory.h"
#include "factories/transformer_factory.h"
#include "factories/connector_factory.h"


Pipeline::Pipeline(std::string const & pipeid, json const & pjson){
    pipeid_ = pipeid;
    last_error_ = "";
    bool success = true;
    // Create connector IN
    try{
        connector_in_ = ConnectorFactory::create(pjson["connector_in"], ConnectorMode::IN, pipeid_);
    }catch(std::exception const & e){
        success = false;
        last_error_ = e.what();
    }catch(...){
        success = false;
        last_error_ = "Unknown exception while creating connector_in";
    }
    // Create filters
    try{
        for (auto filter_parsed : pjson["filters"]) {
            Filter *filter = FilterFactory::create(filter_parsed);
            filters_.push_back(filter);
        }
    }catch(std::exception const & e){
        success = false;
        last_error_ = e.what();
    }catch(...){
        success = false;
        last_error_ = "Unknown exception while creating filters";
    }
    // Create transformers
    try{
        for (auto transformer_parsed : pjson["transformers"]) {
            Transformer *transformer = TransformerFactory::create(transformer_parsed);
            transformers_.push_back(transformer);
        }
    }catch(std::exception const & e){
        success = false;
        last_error_ = e.what();
    }catch(...){
        success = false;
        last_error_ = "Unknown exception while creating transformers";
    }
    // Create connector OUT
    try{
        connector_out_ = ConnectorFactory::create(pjson["connector_out"], ConnectorMode::OUT, pipeid_);
    }catch(std::exception const & e){
        success = false;
        last_error_ = e.what();
    }catch(...){
        success = false;
        last_error_ = "Unknown exception while creating connector_out";
    }

    state_ = success ? PipelineState::STOPPED : PipelineState::MALFORMED;
}


void Pipeline::run(){
    if(state_ == PipelineState::MALFORMED){
        // Pipeline was not configured properly, not safe to run
        return;
    }

    state_ = PipelineState::RUNNING;
    last_error_ = "";
    bool success = true;
    try{
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
    }catch(std::exception const & e){
        success = false;
        last_error_ = e.what();
    }catch(...){
        success = false;
        last_error_ = "Unknown exception while running pipeline";
    }
    state_ = success ? PipelineState::STOPPED : PipelineState::FAILED;
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
    if(th_ != nullptr && state_ == PipelineState::STOPPED){
        th_->join();
        delete th_;
        th_ = nullptr;
    }
    if(th_ == nullptr){
        th_ = new std::thread(&Pipeline::run, this);
    }
}


void Pipeline::stop(){
    stop_ = true;
    connector_in_->stop();  // It helps to exit from blocking receiving call
    if(th_ != nullptr){
        th_->join();
        delete th_;
        th_ = nullptr;
    }
}
