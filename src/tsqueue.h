#ifndef __M2E_BRIDGE_TSQUEUE_H__
#define __M2E_BRIDGE_TSQUEUE_H__


#include <queue>
#include <mutex>
#include <optional>
#include <condition_variable>


template <typename T>
class TSQueue {
public:
    TSQueue():size_limit_(100){};

    void exit_blocking_calls(){
        cv_.notify_one();
    }

    void wait(){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock);
    }

    void push(const T& el){
        std::lock_guard<std::mutex> lock(mtx_);
        if(queue_.size() < size_limit_){
            queue_.push(el);
            cv_.notify_one();
        }
    }

    std::optional<T> pop(){
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        T el = queue_.front();
        queue_.pop();
        return el;
    }

    bool empty()const{
        return queue_.empty();
    }

    size_t size() const {
        return queue_.size();
    }

private:
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::size_t size_limit_;
};


#endif  // __M2E_BRIDGE_TSQUEUE_H__