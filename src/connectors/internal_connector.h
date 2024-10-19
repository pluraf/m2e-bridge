#ifndef __M2E_BRIDGE_INTERNAL_CONNECTOR_H__
#define __M2E_BRIDGE_INTERNAL_CONNECTOR_H__


#include "connector.h"
#include "internal_queues.h"


class InternalConnector: public Connector{
public:
    InternalConnector(std::string pipeid, ConnectorMode mode, json const & config):
            Connector(pipeid, mode, config){
        queuid_ = config.at("name");
    }

    Message receive()override{
        TSQueue<Message> & q = InternalQueues::get_queue(queuid_);
        auto el = q.pop();
        while(! el){
            q.wait();
            el = q.pop();
        }
        return * el;
    }

    void send(Message & msg)override{
        TSQueue<Message> & q = InternalQueues::get_queue(queuid_);
        q.push(msg);
    }

private:
    string queuid_;
};


#endif  // __M2E_BRIDGE_INTERNAL_CONNECTOR_H__