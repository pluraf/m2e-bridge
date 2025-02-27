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


#ifndef __M2E_BRIDGE_TSQUEUE_H__
#define __M2E_BRIDGE_TSQUEUE_H__


#include <memory>
#include <utility>
#include <tuple>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <optional>
#include <limits>
#include <condition_variable>


template <typename T>
class TSQueue {
    std::unordered_map<size_t, std::tuple<std::queue<std::shared_ptr<T>>, size_t> buffers_;
    size_t max_taken_key_ {0};

    std::queue<T> queue_;
    std::mutex mtx_;
    std::mutex bmtx_;
    std::condition_variable cv_;
    size_t size_limit_;
public:
    TSQueue():size_limit_(100){};

    void exit_blocking_calls(){
        cv_.notify_one();
    }

    size_t subscribe(size_t buffer_size=100){
        std::lock_guard<std::mutex> lock(bmtx_);
        if(max_taken_key_ == std::numeric_limits<size_t>::max()){
            throw std::overflow_error("Too many subscribers!");
        }
        buffers_.emplace(++max_taken_key_, std::make_pair({}, buffer_size));
        return max_taken_key_;
    }

    void unsubscribe(size_t subscriber_id){
        std::lock_guard<std::mutex> lock(bmtx_);
        buffers_.erase(subscriber_id);
    }

    void wait(){
        std::unique_lock<std::mutex> lock(mtx_);

        

        cv_.wait(lock);
    }

    void push(T const * el){
        std::lock_guard<std::mutex> lock(mtx_);

        auto ptr = std::shared_ptr<T>{el};

        for (const auto & [subscriber_id, qs] : buffers_){
            auto & [queue, queue_size_max] = qs;
            if(queue.size() < queue_size_max){
                queue.push(ptr);
            }
        }
        cv_.notify_all();
    }

    std::optional<T&> peek(size_t subscriber_id){
        auto & [queue, size] = buffers_.at(subscriber_id);

        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [queue]{ return ! queue.empty() || exit_; });


        return queue_.front();
    }

    void remove(size_t subscriber_id){
        auto & [queue, size] = buffers_.at(subscriber_id);
        if (! queue_.empty()){
            queue_.pop();
        }
    }

    bool empty()const{
        return queue_.empty();
    }

    size_t size() const {
        return queue_.size();
    }
};


#endif  // __M2E_BRIDGE_TSQUEUE_H__