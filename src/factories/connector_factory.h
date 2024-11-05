#ifndef __M2E_BRIDGE_CONNECTOR_FACTORY__
#define __M2E_BRIDGE_CONNECTOR_FACTORY__

#include <iostream>
#include <string>

#include "aws_sdk_manager.h"

#include "connectors/mqtt_connector.h"
#include "connectors/gcp_pubsub_connector.h"
#include "connectors/email_connector.h"
#include "connectors/internal_connector.h"
#include "connectors/gcp_bucket_connector.h"
#include "connectors/aws_s3_connector.h"



class ConnectorFactory{
public:
    static Connector * create(
            std::string const & pipeid, ConnectorMode mode, json const & config){
        string const & conn_type = config.at("type");
        if(conn_type == "mqtt"){
            return new MqttConnector(pipeid, mode, config);
        }else if(conn_type == "gcp_pubsub"){
            return new gcp::PubSubConnector(pipeid, mode, config);
        }else if(conn_type == "email"){
            return new EmailConnector(pipeid, mode, config);
        }else if(conn_type == "queue"){
            return new InternalConnector(pipeid, mode, config);
        }else if(conn_type == "gcp_bucket"){
            return new gcp::CloudStorageConnector(pipeid, mode, config);
        }else if(conn_type == "aws_s3"){
            AwsSdkManager::Instance();
            return new S3Connector(pipeid, mode, config);
        }else{
            throw std::invalid_argument(fmt::format("Unknown Connector type [ {} ]", conn_type));
        }
    }
};


#endif  // __M2E_BRIDGE_CONNECTOR_FACTORY__