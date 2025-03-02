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


#include <queue>
#include <mutex>
#include <optional>
#include <condition_variable>


template <typename T>
class TSQueue{
    size_t max_size_ {0};
    std::queue<T> queue_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool blocking_ {true};

    void check_size(){
        if(max_size_ > 0 && queue_.size() >= max_size_){
             throw std::overflow_error("Queue is full!");
        }
    }
public:
    TSQueue(size_t max_size=0, bool blocking=true){
        max_size_ = max_size;
        blocking_ = blocking;
    }

    ~TSQueue(){
        set_non_blocking();
    }

    void set_non_blocking(){
        blocking_ = false;
        cv_.notify_all();
    }

    void set_blocking(){
        blocking_ = true;
    }

    void push(T const & value){
        std::lock_guard<std::mutex> lock(mtx_);
        check_size();
        queue_.push(value);
        cv_.notify_one();
    }

    void push(T && value){
        std::lock_guard<std::mutex> lock(mtx_);
        check_size();
        queue_.push(value);
        cv_.notify_one();
    }

    T pop(){
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]{ return ! queue_.empty() || ! blocking_; });
        if(queue_.empty()) throw std::underflow_error("No messages in queue!");
        auto el = queue_.front();
        queue_.pop();
        return el;
    }
};



#endif  // __M2E_BRIDGE_TSQUEUE_H__