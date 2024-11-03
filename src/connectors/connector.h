#ifndef __M2E_BRIDGE_CONNECTOR_H__
#define __M2E_BRIDGE_CONNECTOR_H__

#include "nlohmann/json.hpp"
#include <iostream>
#include <string>

#include "m2e_message/message_wrapper.h"
#include "route.h"


enum class ConnectorMode {
    UNKN,
    IN,
    OUT
};


struct ConnectorStat{
    time_t last_in;
    time_t last_out;
    unsigned long count_in;
    unsigned long count_out;
};


class Connector{

protected:
    ConnectorMode mode_ {ConnectorMode::UNKN};
    std::string pipeid_;
    bool is_active_ {false};

public:
    Connector(std::string pipeid, ConnectorMode mode, json const & config){
        mode_ = mode;
        pipeid_ = pipeid;
        is_active_ = true;
    }
    virtual void connect(){}
    virtual void disconnect(){}
    virtual void stop(){is_active_=false;}
    Message receive(){
        Message msg = do_receive();
        ++stat_.count_in;
        auto now = chrono::system_clock::now().time_since_epoch();
        stat_.last_in = chrono::duration_cast<chrono::seconds>(now).count();
        return msg;
    }
    void send(MessageWrapper & msg_w){
        do_send(msg_w);
        ++stat_.count_out;
        auto now = chrono::system_clock::now().time_since_epoch();
        stat_.last_out = chrono::duration_cast<chrono::seconds>(now).count();
    }
    ConnectorStat get_statistics(){
        return stat_;
    }

protected:
    virtual Message do_receive(){return Message();}
    virtual void do_send(MessageWrapper & msg_w){}

private:
    ConnectorStat stat_ {0};
};


#endif  // __M2E_BRIDGE_CONNECTOR_H__