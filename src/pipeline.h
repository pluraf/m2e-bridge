#ifndef __M2E_BRIDGE_PIPELINE_H__
#define __M2E_BRIDGE_PIPELINE_H__


#include <string>
#include <vector>
#include <thread>

#include "m2e_aliases.h"
#include "connectors/connector.h"
#include "filters/filter.h"
#include "transformers/transformer.h"
#include "m2e_message/message_wrapper.h"


enum class PipelineState{
    STOPPED,
    RUNNING,
    FAILED
};

inline std::string pipeline_state_to_string(PipelineState st){
     switch (st) {
        case PipelineState::STOPPED: return "Stopped";
        case PipelineState::RUNNING: return "Running";
        case PipelineState::FAILED: return "Failed";
        default: return "";
    }
}

class Pipeline {
public:
    Pipeline(std::string const & pipeid, json const & pjson);
    void start();
    void stop();
    PipelineState get_state();
    std::string get_id();
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

    bool stop_ {false};
    std::thread *th_ {nullptr};
    PipelineState state_ {PipelineState::STOPPED};
    std::string last_error_;
};


#endif
