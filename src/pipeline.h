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

inline std::string pipeline_state_to_string(PipelineState st){
     switch (st) {
        case PipelineState::STOPPED: return "Stopped";
        case PipelineState::RUNNING: return "Running";
        case PipelineState::FAILED: return "Run Failed";
        case PipelineState::MALFORMED: return "Configuration Malformed";
        case PipelineState::STARTING: return "Starting";
        case PipelineState::STOPPING: return "Stopping";
        default: return "";
    }
}

inline PipelineCommand pipeline_command_from_string(std::string command){
    if(command == "STOP"){
        return PipelineCommand::STOP;
    }
    else if(command == "START"){
        return PipelineCommand::START;
    }
    else if(command == "RESTART"){
        return PipelineCommand::RESTART;
    }
    else{
        return PipelineCommand::UNKN;
    }
}


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

private:
    bool construct(json const & pjson);
    void execute_stop();
    void execute_start();
    void process(Message &msg);
    void process();
    void run_receiving();
    void run_processing();
    void run_sending();
    void run_control();
    void free_resources();

    Connector * connector_in_;
    Connector * connector_out_;
    std::vector<Filtra*> filtras_;
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
    TSQueue<Message> s_queue_;
    TSQueue<PipelineCommand> c_queue_;
    TSQueue<bool> e_queue_;

    bool is_alive_;
    bool is_active_;

};


#endif
