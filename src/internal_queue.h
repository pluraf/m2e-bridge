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


#ifndef __M2E_BRIDGE_INTERNAL_QUEUES_H__
#define __M2E_BRIDGE_INTERNAL_QUEUES_H__


#include <memory>
#include <utility>
#include <tuple>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <map>
#include <string>
#include <shared_mutex>
#include <optional>
#include <limits>
#include <stdexcept>
#include <condition_variable>

#include "m2e_message/message.h"
#include "tsqueue.h"


class InternalQueue;


class RQueue{
    size_t id_ {};
    TSQueue<std::shared_ptr<Message>> * queue_ {nullptr};
public:
    RQueue() = default;
    RQueue(size_t id, TSQueue<std::shared_ptr<Message>> * queue): id_(id), queue_(queue) {}

    RQueue(RQueue const & other): id_(other.id_), queue_(other.queue_) {}

    std::shared_ptr<Message> pop(){
        return queue_->pop();
    }

    friend class InternalQueue;
};


class InternalQueue {
    std::unordered_map<size_t, TSQueue<std::shared_ptr<Message>>> buffers_;
    size_t max_taken_key_ {0};
    std::shared_mutex buffers_mtx_;
    static std::map<std::string, InternalQueue> queues_;  // queuid

    void exit_blocking_calls(){
        for (auto & [subscriber_id, buffer] : buffers_){
            buffer.set_non_blocking();
        }
    }
public:
    InternalQueue(InternalQueue const & other) = delete;
    InternalQueue() = default;

    static InternalQueue & get_queue(std::string const & queuid){
        return queues_[queuid];
    }

    static InternalQueue * get_queue_ptr(std::string const & queuid){
        return & queues_[queuid];
    }

    static void redirect(Message const & msg, std::string const & queuid){
        get_queue(queuid).push(std::make_shared<Message>(msg));
    }

    RQueue subscribe(size_t buffer_size){
        std::lock_guard<std::shared_mutex> lock(buffers_mtx_);
        if(max_taken_key_ == std::numeric_limits<size_t>::max()){
            throw std::overflow_error("Too many subscribers!");
        }
        auto inserted = buffers_.emplace(++max_taken_key_, buffer_size);
        if(inserted.second) return RQueue(max_taken_key_, & (*inserted.first).second);
        throw std::runtime_error("Can not subscribe!");
    }

    void unsubscribe(RQueue const & rq){
        std::lock_guard<std::shared_mutex> lock(buffers_mtx_);
        buffers_.erase(rq.id_);
    }

    void push(std::shared_ptr<Message> const & msg_ptr){
        std::shared_lock<std::shared_mutex> lock(buffers_mtx_);
        for (auto & [subscriber_id, buffer] : buffers_){
            buffer.push(msg_ptr);
        }
    }

    void push(Message && msg){
        auto msg_ptr = std::make_shared<Message>(msg);
        std::shared_lock<std::shared_mutex> lock(buffers_mtx_);
        for (auto & [subscriber_id, buffer] : buffers_){
            buffer.push(msg_ptr);
        }
    }
};


#endif  // __M2E_BRIDGE_INTERNAL_QUEUES_H__