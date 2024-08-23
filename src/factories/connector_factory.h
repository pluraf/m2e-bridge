
#include "nlohmann/json.hpp"
#include <iostream>

#include "connectors/connector.h"
#include "connectors/mqtt_connector.h"


class ConnectorFactory {
public:
    static Connector* create(nlohmann::json json_descr, std::string type) {
        std::cout<< "connector type: "<< json_descr["type"]<< std::endl;
        if (json_descr["type"] == "mqtt"){
            return new MqttConnector(json_descr, type);
                
        }
        else {
            return new Connector(json_descr, type);
        }     
    }
};