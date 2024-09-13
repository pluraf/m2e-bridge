#ifndef __M2E_BRIDGE_GLOBAL_STATE_H__
#define __M2E_BRIDGE_GLOBAL_STATE_H__


#include <vector>
#include <mutex>
#include <functional>


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

private:
    std::mutex exit_container_mutex_;
    std::vector<Callback> exit_callbacks_;
};


extern GlobalState gs;


#endif  // __M2E_BRIDGE_GLOBAL_STATE_H__