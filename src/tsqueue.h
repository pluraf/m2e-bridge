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