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
    state_ = success ? PipelineState::STOPPED : PipelineState::MALFORMED;
    // start control thread
    control_thread_ = new std::thread(&Pipeline::run_control_thread, this);
}

 Pipeline::~Pipeline(){
    pipeline_exit_ = true;
    {
        std::unique_lock<std::mutex> lock(command_mutex_);
        command_ = PipelineCommand::NONE;
        new_command_condition_.notify_one();  
    }

    if(control_thread_ != nullptr){
        control_thread_->join();
        delete control_thread_;
        control_thread_ = nullptr;
    }
 }


void Pipeline::run(){
    if(state_ == PipelineState::MALFORMED){
        // Pipeline was not configured properly, not safe to run
        return;
    }
    last_error_ = "";
    bool success = true;
    try{
        connector_in_->connect();
        connector_out_-> connect();
        state_ = PipelineState::RUNNING;
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
    state_ = success ? PipelineState::STOPPED : PipelineState::FAILED;
}

//private function to start pipeline in control thread
void Pipeline::execute_start(){
    if(state_ == PipelineState::MALFORMED || 
        state_ == PipelineState::RUNNING
        || state_ == PipelineState::STARTING){
        return;
    }
    PipelineState prev_state = state_;
    state_ = PipelineState::STARTING;
    if(run_thread_ != nullptr && 
        prev_state == PipelineState::FAILED){
        run_thread_->join();
        delete run_thread_;
        run_thread_ = nullptr;
    }
    if(run_thread_ == nullptr){
        stop_ = false;
        run_thread_ = new std::thread(&Pipeline::run, this);
    }
}

//private function to stop pipeline in control thread
void Pipeline::execute_stop(){
    if(state_ == PipelineState::MALFORMED || state_ == PipelineState::STOPPED){
        return;
    }
    state_ = PipelineState::STOPPING;
    std::cout<<"Stopping pipeline : "<<pipeid_<<std::endl;
    stop_ = true;
    if(connector_in_ != nullptr){
        connector_in_->stop();  // It helps to exit from blocking receiving call
    }   
    if(run_thread_ != nullptr){
        run_thread_->join();
        delete run_thread_;
        run_thread_ = nullptr;
    }
    state_= PipelineState::STOPPED;
}

void Pipeline::run_control_thread(){
    while(true){
        std::unique_lock<std::mutex> lock(command_mutex_);
        new_command_condition_.wait(lock);
        if(command_ == PipelineCommand::NONE && pipeline_exit_){
            break;
        }
        switch(command_){
            case PipelineCommand::START:
                execute_start();
                break;
            case PipelineCommand::STOP:
                execute_stop();
                break;
            case PipelineCommand::RESTART:
                execute_stop();
                execute_start();
                break;
            default:
                break;
        }
    }
    
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

void Pipeline::give_command(PipelineCommand command){
    // setting command_ and notifying condition variable new_command_condition_
    // will make control thread wake up and execute the command
    std::unique_lock<std::mutex> lock(command_mutex_);
    command_ = command;
    new_command_condition_.notify_one(); 
}

void Pipeline::start() {
    give_command(PipelineCommand::START);
}

void Pipeline::restart() {
    give_command(PipelineCommand::RESTART);
}

void Pipeline::stop(){
    give_command(PipelineCommand::STOP);
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