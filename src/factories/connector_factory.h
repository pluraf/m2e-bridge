
#ifndef __M2E_BRIDGE_CONNECTOR_FACTORY__
#define __M2E_BRIDGE_CONNECTOR_FACTORY__

#include "nlohmann/json.hpp"
#include <iostream>

#include "connectors/connector.h"
#include "connectors/mqtt_connector.h"
#include "connectors/gcp_pubsub_connector.h"


class ConnectorFactory {
public:
    static Connector* create(nlohmann::json json_descr, std::string type) {
        std::cout<< "connector type: "<< json_descr["type"]<< std::endl;
        if (json_descr["type"] == "mqtt"){
            return new MqttConnector(json_descr, type);
                
        }
        else if (json_descr["type"] == "gcp_pubsub"){
            return new gcp::PubSubConnector(json_descr, type);
                
        }
        else {
            return new Connector(json_descr, type);
        }     
    }
};

#endif