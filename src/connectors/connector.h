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
    virtual Message receive(){return Message();}
    virtual void send(Message & msg){}
};


#endif  // __M2E_BRIDGE_CONNECTOR_H__