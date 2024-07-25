/*
Copyright Pluraf Embedded AB, 2024
code@pluraf.com
*/


#include "pipeline.h"


void Pipeline::run() {
    connector_in_.start();

    while (!stop_) {
        Message msg = connector_in_.receive();
        MessageWrapper msg_w = MessageWrapper(msg);
        if (! filter(msg_w)) {
            continue;
        }
        transform(msg_w);
        map(msg_w);
    }

    connector_in_.stop();
}


bool Pipeline::filter(MessageWrapper& msg_w) {
    for (auto &filter : filters_) {
        if (! filter.apply(MessageWrapper)) {
            return false;
        }
    }
    return true;
}


void Pipeline::transform(MessageWrapper& msg_w) {
    for (auto &transformer : transformers_) {
        transformer.apply(MessageWrapper);
    }
}


void Pipeline::map(MessageWrapper& msg_w) {
    for (auto &mapper : mappers_) {
        mapper.map(msg_w);
    }
}


void Pipeline::start() {
    if (th_ == nullptr) {
        th_ = new thread(&CANSender::sendCyclicly, this, fc_ptr);
    }
}


void Pipeline::stop() {
    if (th_ != nullptr) {
        stop_ = true;
        th_->join();
        delete th_;
        th_ = nullptr;
    }
}
