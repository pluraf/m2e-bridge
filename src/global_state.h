#ifndef __M2E_BRIDGE_GLOBAL_STATE_H__
#define __M2E_BRIDGE_GLOBAL_STATE_H__


#include <vector>
#include <mutex>
#include <functional>

#include "pipeline_supervisor.h"


class GlobalState {
public:
    using Callback = std::function<void()>;

    void register_exit_cb(Callback cb){
        std::lock_guard<std::mutex> lock(exit_container_mutex_);
        exit_callbacks_.push_back(cb);
    }

    void notify_exit(){
        for(const auto & cb: exit_callbacks_){
            cb();
        }
    }

    PipelineSupervisor* get_pipeline_supervisor(){
        if (supervisor_ptr_ == nullptr){
            supervisor_ptr_ = new PipelineSupervisor();
            supervisor_ptr_->init();
        }
        return supervisor_ptr_;
    }

private:
    std::mutex exit_container_mutex_;
    std::vector<Callback> exit_callbacks_;
    PipelineSupervisor* supervisor_ptr_;
};


extern GlobalState gs;


#endif  // __M2E_BRIDGE_GLOBAL_STATE_H__