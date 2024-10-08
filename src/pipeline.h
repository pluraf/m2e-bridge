#ifndef __M2E_BRIDGE_PIPELINE_H__
#define __M2E_BRIDGE_PIPELINE_H__


#include <string>
#include <vector>
#include <thread>
#include <atomic>

#include "m2e_aliases.h"
#include "connectors/connector.h"
#include "filters/filter.h"
#include "transformers/transformer.h"
#include "m2e_message/message_wrapper.h"


enum class PipelineState{
    STOPPED,
    RUNNING,
    MALFORMED,
    FAILED
};

enum class PipelineCommand{
    STOP,
    START,
    RESTART,
    NONE
};

inline std::string pipeline_state_to_string(PipelineState st){
     switch (st) {
        case PipelineState::STOPPED: return "Stopped";
        case PipelineState::RUNNING: return "Running";
        case PipelineState::FAILED: return "Run Failed";
        case PipelineState::MALFORMED: return "Configuration Malformed";
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
        return PipelineCommand::NONE;
    }
}

//Since Pipeline class contain an atomic bool member (stop_) 
//Copy constructor and copy assignment are not allowed
//stop_ is made atomic bool to make it safely accessible from both pipeline run thread and the main thread
class Pipeline {
public:
    Pipeline(std::string const & pipeid, json const & pjson);
    void start();
    void stop();
    void restart();
    PipelineState get_state() const;
    std::string get_id() const;
    std::string get_last_error() const;

    // Delete copy constructor and copy assignment operator because of atomic bool member
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    Pipeline& operator=(Pipeline&&) = delete;

    //Custom move constructor because of atomic bool member
    Pipeline(Pipeline&& other) noexcept
        : stop_(other.stop_.load()),
        connector_in_(std::move(other.connector_in_)),
        connector_out_(std::move(other.connector_out_)),
        filters_(std::move(other.filters_)),
        transformers_(std::move(other.transformers_)),
        pipeid_(std::move(other.pipeid_)),
        th_(std::move(other.th_)),
        state_(std::move(other.state_)),
        last_error_(std::move(other.last_error_))
        
    {
        other.stop_.store(false);
    }
private:
    void run();
    bool filter(MessageWrapper& msg_w);
    void transform(MessageWrapper& msg_w);
    void map(MessageWrapper& msg_w);
    Connector* connector_in_;
    Connector* connector_out_;
    std::vector<Filter*> filters_;
    std::vector<Transformer*> transformers_;
    std::string pipeid_;

    std::atomic<bool> stop_; //atomic to make it thread safe
    std::thread *th_ {nullptr};
    PipelineState state_ {PipelineState::STOPPED};
    std::string last_error_;
};


#endif
