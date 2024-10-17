
#ifndef __M2E_BRIDGE_CONNECTOR_FACTORY__
#define __M2E_BRIDGE_CONNECTOR_FACTORY__

#include <iostream>
#include <string>

#include "connectors/mqtt_connector.h"
#include "connectors/gcp_pubsub_connector.h"


class ConnectorFactory {
public:
    static Connector * create(
        json const & json_descr, ConnectorMode mode, std::string const & pipeid
    ){
        std::cout<< "connector type: "<< json_descr["type"]<< std::endl;
        if (json_descr["type"] == "mqtt"){
            return new MqttConnector(json_descr, mode, pipeid);
        }
        else if (json_descr["type"] == "gcp_pubsub"){
            return new gcp::PubSubConnector(json_descr, mode, pipeid);
        }
        else {
            return new Connector(json_descr, mode, pipeid);
        }
    }
};

#endif