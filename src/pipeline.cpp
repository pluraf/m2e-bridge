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
#include "internal_queue.h"
#include "factories/filtra_factory.h"
#include "factories/connector_factory.h"


Pipeline::Pipeline(std::string const & pipeid, json const & pjson){
    pipeid_ = pipeid;
    config_ = pjson;
    prepare();
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
    // Enabled
    is_enabled_ = pjson.value("enabled", false);
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


 bool Pipeline::prepare(){
    bool success = construct(config_);
    if(success){
        state_ = PipelineState::STOPPED;
        control_thread_ = new std::thread(& Pipeline::run_control, this);
    }else{
        state_ = PipelineState::MALFORMED;
        free_resources();  // perform cleaning
    }
    return success;
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
    int filtra_ix = 0;
    Message msg;
    while(filtra_ix < filtras_.size()){
        auto msg = filtras_[filtra_ix]->process();
        if(msg) break;
        ++filtra_ix;
    }
    if(msg){
        std::shared_ptr<Message> msg_ptr = std::make_shared<Message>(std::move(msg));
        MessageWrapper msg_w(msg_ptr);
        if(msg_w.is_passed()){
            for(auto const & queuid : filtras_[filtra_ix]->get_destinations()){
                InternalQueue::redirect(msg_w.msg(), queuid);
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
                    InternalQueue::redirect(msg_w.msg(), queuid);
                }
            }
        }
        if(msg_w.is_passed()) s_queue_.push(msg_w);
    }
}


void Pipeline::process(std::shared_ptr<Message> const & msg_ptr, int filtra_ix){
    MessageWrapper msg_w(msg_ptr);
    while(filtra_ix < filtras_.size() && is_active()){
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
                process(std::make_shared<Message>(new_msg), filtra_ix);
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
                InternalQueue::redirect(msg_w.msg(), queuid);
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


void Pipeline::handle_events(){
    while(is_active()){
        try{
            e_queue_.pop();
            process();
        }catch(std::underflow_error){
            break;
        }
    }
}


void Pipeline::handle_messages(){
    while(is_active()){
        try{
            auto msg_ptr = r_queue_.pop();
            process(std::move(msg_ptr));
        }catch(std::underflow_error){
            break;
        }
    }
}


void Pipeline::run_receiving(ThreadState * thread_state){
    * thread_state = ThreadState::STARTING;
    // Connect
    try{
        connector_in_->connect();
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
    // Receiving forever
    if(state_ != PipelineState::FAILED){
        * thread_state = ThreadState::RUNNING;
        std::cout << state_to_string(state_) << std::endl;
        while(is_active()){
            try{
                auto msg_ptr = connector_in_->receive();
                r_queue_.push(std::move(msg_ptr));
                pipeline_event_.notify_one();
            }catch(std::underflow_error){
                break;
            }catch(std::exception const & e){
                last_error_ = e.what();
                // sleep to prevent while(1) flood (temp.)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    }
    // Disconnect
    try{
        connector_in_->disconnect();
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
    std::cout << "run_receiving finished" << std::endl;
    * thread_state = ThreadState::FINISHED;
}


void Pipeline::run_processing(ThreadState * thread_state){
    * thread_state = ThreadState::STARTING;
    try{
        for(auto filtra : filtras_) filtra->start();
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
    * thread_state = ThreadState::RUNNING;
    try{
        while(is_active()){
            handle_events();
            handle_messages();
            std::unique_lock<std::mutex> lock(pipeline_event_mtx_);
            pipeline_event_.wait(lock);
        }
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
    std::cout << "run_processing finished" << std::endl;
    * thread_state = ThreadState::FINISHED;
}


void Pipeline::run_sending(ThreadState * thread_state){
    * thread_state = ThreadState::STARTING;
    // Connect
    try{
        connector_out_->connect();
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
    // Sending forever
    if(state_ != PipelineState::FAILED){
        * thread_state = ThreadState::RUNNING;
        while(is_active()){
            try{
                auto msg_w = s_queue_.pop();
                connector_out_->send(msg_w);
            }catch(std::underflow_error){
                break;
            }catch(std::exception const & e){
                last_error_ = e.what();
                // sleep to prevent while(1) flood (temp.)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            last_error_ = "";  // Clear error on successful sending
        }
    }
    // Disconnect
    try{
        connector_out_->disconnect();
    }catch(std::exception const & e){
        last_error_ = e.what();
        state_ = PipelineState::FAILED;
    }
    std::cout << "run_sending finished" << std::endl;
    * thread_state = ThreadState::FINISHED;
}


void Pipeline::run_control(){
    while(is_alive()){
        std::optional<PipelineCommand> command;
        try{
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
                        execute_stop();
                        state_ = PipelineState::TERMINATED;
                        break;
                    default:
                        break;
                }
            }
        }catch(std::underflow_error){
            execute_stop();
            state_ = PipelineState::TERMINATED;
        }catch(std::exception const & e){
            last_error_ = e.what();
        }
    }
    std::cout<<pipeid_<<": control thread terminated"<<std::endl;
    free_resources();
}


void Pipeline::execute_start(){
    if(state_ == PipelineState::RUNNING || state_ == PipelineState::STARTING) return;
    if(state_ != PipelineState::STOPPED){
        execute_stop();
    }
    last_error_ = "";
    state_ = PipelineState::RUNNING;

    s_queue_.set_blocking();

    sending_thread_.start(& Pipeline::run_sending, this);
    processing_thread_.start(& Pipeline::run_processing, this);
    receiving_thread_.start(& Pipeline::run_receiving, this);
}


void Pipeline::execute_stop(){
    if(state_ == PipelineState::MALFORMED
            || state_ == PipelineState::STOPPED
            || state_ == PipelineState::STOPPING){
        return;
    }

    state_ = PipelineState::STOPPING;

    // It helps to exit from blocking receiving call
    if(connector_in_ != nullptr) connector_in_->stop();
    s_queue_.set_non_blocking();
    receiving_thread_.terminate();
    while(processing_thread_.is_running()){
        pipeline_event_.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    processing_thread_.terminate();
    sending_thread_.terminate();

    state_ = PipelineState::STOPPED;
}


void Pipeline::execute(PipelineCommand cmd){
    if(! is_alive() && ! prepare()) return;
    c_queue_.push(cmd);
}


void Pipeline::start() {
    is_enabled_ = true;
    execute(PipelineCommand::START);
}


void Pipeline::restart() {
    is_enabled_ = true;
    execute(PipelineCommand::RESTART);
}


void Pipeline::stop(){
    is_enabled_ = false;
    execute(PipelineCommand::STOP);
}


void Pipeline::terminate(){
    execute(PipelineCommand::TERMINATE);
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
