#ifndef __M2E_BRIDGE_INTERNAL_QUEUES_H__
#define __M2E_BRIDGE_INTERNAL_QUEUES_H__


#include "tsqueue.h"
#include "m2e_message/message_wrapper.h"


class InternalQueues{
    InternalQueues(){} // private constructor to make class singleton
    static InternalQueues * instance_;
    static map<string, TSQueue<Message>> queues_;  // queuid

public:
    static InternalQueues * get_instance(){
        if (instance_ == nullptr){
            instance_ = new InternalQueues();
        }
        return instance_;
    }

    static void redirect(MessageWrapper & msg_w, string const & queuid){
        Message const & msg = msg_w.msg();
        TSQueue<Message> & queue = queues_[queuid];
        queue.push(msg);
    }

    static TSQueue<Message> & get_queue(string const & queuid){
        return queues_[queuid];
    }
};


#endif // __M2E_BRIDGE_INTERNAL_QUEUES_H__