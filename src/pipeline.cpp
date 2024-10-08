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


Pipeline::Pipeline(std::string const & pipeid, json const & pjson): stop_(false){
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

    state_ = success ? PipelineState::STOPPED : PipelineState::CONFIG_FAILED;
}


void Pipeline::run() {
    state_ = PipelineState::RUNNING;
    last_error_ = "";
    bool success = true;
    try{
        connector_in_->connect();
        connector_out_-> connect();
        while(! stop_){
            MessageWrapper* msg_w = connector_in_->receive();
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
    state_ = success ? PipelineState::STOPPED : PipelineState::RUN_FAILED;
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
    if(state_ == PipelineState::CONFIG_FAILED){
        return;
    }
    if(th_ != nullptr && 
        (state_ == PipelineState::STOPPED || state_ == PipelineState::RUN_FAILED)){
        th_->join();
        delete th_;
        th_ = nullptr;
    }
    if(th_ == nullptr){
        stop_ = false;
        th_ = new std::thread(&Pipeline::run, this);
    }
}

void Pipeline::restart() {
    stop();
    start();
}

void Pipeline::stop(){
    std::cout<<"Called stop for pipeid "<<pipeid_<<std::endl;
    stop_ = true;
    if(connector_in_ != nullptr){
        connector_in_->stop();  // It helps to exit from blocking receiving call
    }   
    if(th_ != nullptr){
        th_->join();
        delete th_;
        th_ = nullptr;
    }
}

PipelineState Pipeline::get_state() const{
    return state_;
}

std::string Pipeline::get_id() const{
    return pipeid_;
}

std::string Pipeline::get_last_error() const{
    return last_error_;
}