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

    static void redirect_if(MessageWrapper & msg_w){
        Message const & msg = msg_w.msg();
        for(string const & queuid : msg_w.get_destinations()){
            TSQueue<Message> & queue = queues_[queuid];
            queue.push(msg);
        }
        msg_w.clear_destinations();
    }
};


#endif // __M2E_BRIDGE_INTERNAL_QUEUES_H__