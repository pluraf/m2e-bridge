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


#ifndef __M2E_BRIDGE_PIPELINE_H__
#define __M2E_BRIDGE_PIPELINE_H__


#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "m2e_aliases.h"
#include "pipeline_iface.h"
#include "tsqueue.h"
#include "filtra.h"
#include "connectors/connector.h"
#include "m2e_message/message_wrapper.h"


enum class PipelineState{
    UNKN,
    STOPPED,
    RUNNING,
    MALFORMED,
    FAILED,
    STARTING,
    STOPPING
};

enum class PipelineCommand{
    UNKN,
    STOP,
    START,
    RESTART,
    TERMINATE
};


struct PipelineStat{
    time_t last_in;
    time_t last_out;
    unsigned long count_in;
    unsigned long count_out;
};


class Pipeline:public PipelineIface{
public:
    Pipeline(std::string const & pipeid, json const & pjson);
    ~Pipeline();

    Pipeline(const Pipeline &) = delete;
    Pipeline(Pipeline &&) = delete;
    Pipeline & operator=(const Pipeline &) = delete;
    Pipeline & operator=(Pipeline &&) = delete;

    void schedule_event(std::chrono::milliseconds msec);

    void execute(PipelineCommand cmd);
    void start();
    void stop();
    void restart();
    void terminate();

    PipelineState get_state() const;
    std::string get_id() const;
    std::string get_last_error() const;

    PipelineStat get_statistics()const{
        PipelineStat stat = {};
        if(connector_in_){
            stat.last_in = connector_in_->get_statistics().last_in;
            stat.count_in = connector_in_->get_statistics().count_in;
        }
        if(connector_out_){
            stat.last_out = connector_out_->get_statistics().last_out;
            stat.count_out = connector_out_->get_statistics().count_out;
        }
        return stat;
    }

    static string state_to_string(PipelineState st){
        switch (st) {
            case PipelineState::STOPPED: return "stopped";
            case PipelineState::RUNNING: return "running";
            case PipelineState::FAILED: return "failed";
            case PipelineState::MALFORMED: return "malformed";
            case PipelineState::STARTING: return "starting";
            case PipelineState::STOPPING: return "stopping";
            case PipelineState::UNKN: return "unknown";
        }
    }

    static PipelineCommand command_from_string(string command){
        if(command == "stop") return PipelineCommand::STOP;
        if(command == "start") return PipelineCommand::START;
        if(command == "restart") return PipelineCommand::RESTART;
        if(command == "terminate") return PipelineCommand::TERMINATE;
        return PipelineCommand::UNKN;
    }

private:
    bool construct(json const & pjson);
    void execute_stop();
    void execute_start();
    void process(Message const & msg, int filtra_ix = 0);
    void process();
    void run_receiving();
    void run_processing();
    void run_sending();
    void run_control();
    void free_resources();
    int find_filtra_index(string const & filtra_name);

    Connector * connector_in_ {nullptr};
    Connector * connector_out_ {nullptr};
    std::vector<Filtra *> filtras_;
    std::string pipeid_;

    std::thread * processing_thread_ {nullptr};
    std::thread * receiving_thread_ {nullptr};
    std::thread * sending_thread_ {nullptr};
    std::thread * control_thread_ {nullptr};
    PipelineState state_ {PipelineState::STOPPED};
    std::string last_error_;

    std::mutex pipeline_event_mtx_;
    std::condition_variable pipeline_event_;

    TSQueue<Message> r_queue_;
    TSQueue<MessageWrapper> s_queue_;
    TSQueue<PipelineCommand> c_queue_;
    TSQueue<bool> e_queue_;

    bool is_alive_;
    bool is_active_;

};


#endif
