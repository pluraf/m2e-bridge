/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


#include <fstream>
#include <iostream>
#include <stdexcept>

#include "global_state.h"
#include "pipeline.h"
#include "tsqueue.h"
#include "internal_queues.h"
#include "factories/filtra_factory.h"
#include "factories/connector_factory.h"


Pipeline::Pipeline(std::string const & pipeid, json const & pjson){
    pipeid_ = pipeid;
    bool success = construct(pjson);
    if(success){
        state_ = PipelineState::STOPPED;
        is_alive_ = true;
        control_thread_ = new std::thread(& Pipeline::run_control, this);
    }else{
        state_ = PipelineState::MALFORMED;
        // perform cleaning
        free_resources();
    }
}


Pipeline::~Pipeline(){
    if(state_ != PipelineState::MALFORMED){
        terminate();
        control_thread_->join();
        delete control_thread_;
        control_thread_ = nullptr;
    }
}


bool Pipeline::construct(json const & pjson){
    last_error_ = "";
    bool success = true;
    // Create connector IN
    try{
        connector_in_ = ConnectorFactory::create(
            pipeid_, ConnectorMode::IN, pjson.at("connector_in")
        );
    }catch(std::exception const & e){
        last_error_ = e.what();
        return false;
    }
    // Create filtras
    if(pjson.contains("filtras")){
        try{
            for(auto filtra_parsed : pjson["filtras"]){
                Filtra * filtra = FiltraFactory::create(* this, filtra_parsed);
                filtras_.push_back(filtra);
            }
        }catch(std::exception const & e){
            last_error_ = e.what();
            return false;
        }
    }
    // Create connector OUT
    try{
        connector_out_ = ConnectorFactory::create(
            pipeid_, ConnectorMode::OUT, pjson["connector_out"]
        );
    }catch(std::exception const & e){
        last_error_ = e.what();
        return false;
    }
    return true;
 }


void Pipeline::free_resources(){
    if(connector_in_){
        delete connector_in_;
        connector_in_ = nullptr;
    }
    if(connector_out_){
        delete connector_out_;
        connector_out_ = nullptr;
    }
    for(Filtra * filtra : filtras_){
        delete filtra;
    }
    filtras_.clear();
}


int Pipeline::find_filtra_index(string const & filtra_name){

    if(filtra_name == "out") return filtras_.size();

    for(int ix = 0; ix < filtras_.size(); ++ix){
        if(filtras_[ix]->get_name() == filtra_name){
            return ix;
        }
    }
    return filtras_.size();
}


void Pipeline::process(){
    bool is_passed = true;
    MessageWrapper msg_w;
    int filtra_ix = 0;
    while(filtra_ix < filtras_.size()){
        msg_w = filtras_[filtra_ix]->process();
        if(msg_w) break;
        ++filtra_ix;
    }

    if(msg_w){
        if(msg_w.is_passed()){
            for(auto const & queuid : filtras_[filtra_ix]->get_destinations()){
                InternalQueues::redirect(msg_w, queuid);
            }
        }
        ++filtra_ix;
        for(; msg_w.is_passed() && filtra_ix < filtras_.size(); filtra_ix++){
            try{
                filtras_[filtra_ix]->process(msg_w);
            }catch(std::exception const & e){
                last_error_ = e.what();
                return;
            }
            if(msg_w.is_passed()){
                for(auto const & queuid : filtras_[filtra_ix]->get_destinations()){
                    InternalQueues::redirect(msg_w, queuid);
                }
            }
        }
        if(msg_w.is_passed()) s_queue_.push(msg_w);
    }
}


void Pipeline::process(Message const & msg, int filtra_ix){
    MessageWrapper msg_w(msg);
    while(filtra_ix < filtras_.size() && is_active_){

        Filtra * filtra = nullptr;
        try{
            filtra = filtras_.at(filtra_ix);
        }catch(std::out_of_range){
            msg_w.reject();
            break;
        }

        string hop;
        try{
            hop = filtra->process(msg_w);
        }catch(std::invalid_argument){
            msg_w.reject();
            break;
        }catch(std::exception const & e){
            last_error_ = e.what();
            return;
        }

        if(hop == "self"){
            hops_t hops = filtra->get_hops();
            if(! hops.first.empty()){
                filtra_ix = find_filtra_index(hops.first);
            }else{
                ++filtra_ix;
            }
            Message new_msg = filtra->process();
            while(new_msg){
                process(new_msg, filtra_ix);
                new_msg = filtra->process();
            }
            return;
        }

        std::pair<string, string> hops = filtra->get_hops();
        if(msg_w.is_passed()){
            if(! hops.first.empty()){
                filtra_ix = find_filtra_index(hops.first);
            }else{
                filtra_ix++;
            }
            for(auto const & queuid : filtra->get_destinations()){
                InternalQueues::redirect(msg_w, queuid);
            }
        }else{
            if(! hops.second.empty()){
                filtra_ix = find_filtra_index(hops.second);
                msg_w.pass(); // let message to flow forward
            }else{
                break;
            }
        }
    }
    if(msg_w.is_passed()) s_queue_.push(msg_w);
}


void Pipeline::run_receiving(){
    try{
        Message msg;
        connector_in_->connect();
        while(is_active_){
            try{
                msg = connector_in_->receive();
            }catch(std::out_of_range){
                continue;
            }
            r_queue_.push(std::move(msg));
            pipeline_event_.notify_one();
        }
        connector_in_->disconnect();
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
}


void Pipeline::run_processing(){
    try{
        while(is_active_){
            while(e_queue_.pop()){
                process();
            }
            std::optional<Message> el;
            while((el = r_queue_.pop())){
                if(el){
                    process(* el);
                }
            }
            std::unique_lock<std::mutex> lock(pipeline_event_mtx_);
            pipeline_event_.wait(lock);
        }
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
}


void Pipeline::run_sending(){
    try{
        connector_out_->connect();
        while(is_active_){
            std::optional<MessageWrapper> el;
            while((el = s_queue_.pop())){
                connector_out_->send(* el);
            }
            s_queue_.wait();
        }
        connector_out_->disconnect();
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
}


void Pipeline::run_control(){
    while(is_alive_){
        std::optional<PipelineCommand> command;
        while((command = c_queue_.pop())){
            switch(* command){
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
                case PipelineCommand::TERMINATE:
                    is_alive_ = false;
                    execute_stop();
                    break;
                default:
                    break;
            }
        }
        if(is_alive_){
            c_queue_.wait();
        }else{
            break;
        }
    }
    std::cout<<pipeid_<<": control thread terminated"<<std::endl;
    free_resources();
}


void Pipeline::execute_start(){
    if(state_ != PipelineState::STOPPED){
        return;
    }
    last_error_ = "";
    is_active_ = true;

    sending_thread_ = new std::thread(& Pipeline::run_sending, this);
    processing_thread_ = new std::thread(& Pipeline::run_processing, this);
    receiving_thread_ = new std::thread(& Pipeline::run_receiving, this);

    state_ = PipelineState::RUNNING;
}


void Pipeline::execute_stop(){
    if(state_ == PipelineState::MALFORMED
            || state_ == PipelineState::STOPPED
            || state_ == PipelineState::STOPPING){
        return;
    }

    is_active_ = false;
    state_ = PipelineState::STOPPING;

    // It helps to exit from blocking receiving call
    try{
        if(connector_in_ != nullptr) connector_in_->stop();
    }catch(std::exception const & e){
        last_error_ = e.what();
    }
    s_queue_.exit_blocking_calls();

    std::cout<<pipeid_<<": waiting for receiving_thread to finish..."<<std::endl;
    receiving_thread_->join();
    pipeline_event_.notify_all();
    std::cout<<pipeid_<<": waiting for processing_thread to finish..."<<std::endl;
    processing_thread_->join();
    std::cout<<pipeid_<<": waiting for sending_thread to finish..."<<std::endl;
    sending_thread_->join();
    std::cout<<pipeid_<<": stopped"<<std::endl;

    delete receiving_thread_;
    delete processing_thread_;
    delete sending_thread_;

    state_= PipelineState::STOPPED;
}


void Pipeline::execute(PipelineCommand cmd){
    c_queue_.push(cmd);
}


void Pipeline::start() {
    c_queue_.push(PipelineCommand::START);
}


void Pipeline::restart() {
    c_queue_.push(PipelineCommand::RESTART);
}


void Pipeline::stop(){
    c_queue_.push(PipelineCommand::STOP);
}


void Pipeline::terminate(){
    c_queue_.push(PipelineCommand::TERMINATE);
}


PipelineState Pipeline::get_state()const{
    return state_;
}


std::string Pipeline::get_id()const{
    return pipeid_;
}


std::string Pipeline::get_last_error() const{
    return last_error_;
}


void Pipeline::schedule_event(std::chrono::milliseconds msec){
    std::thread([this, msec](){
        std::this_thread::sleep_for(std::chrono::milliseconds(msec));
        this->e_queue_.push(true);
        this->pipeline_event_.notify_one();
    }).detach();
}