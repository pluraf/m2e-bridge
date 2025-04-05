/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


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
#include "connectors/http_connector.h"
#include "connectors/azure_service_bus_connector.h"
#include "connectors/slack_connector.h"
#include "connectors/generator_connector.h"


class ConnectorFactory{
public:
    static Connector * create(
            std::string const & pipeid, ConnectorMode mode, json const & config){
        string const & conn_type = config.at("type");
        if(conn_type == "mqtt"){
            return new MqttConnector(pipeid, mode, config);
        }else if(conn_type == "gcp_pubsub"){
            return new PubSubConnector(pipeid, mode, config);
        }else if(conn_type == "email"){
            return new EmailConnector(pipeid, mode, config);
        }else if(conn_type == "queue"){
            return new InternalConnector(pipeid, mode, config);
        }else if(conn_type == "gcp_bucket"){
            return new CloudStorageConnector(pipeid, mode, config);
        }else if(conn_type == "aws_s3"){
            AwsSdkManager::Instance();
            return new S3Connector(pipeid, mode, config);
        }else if(conn_type == "http"){
            return new HttpConnector(pipeid, mode, config);
        }else if(conn_type == "azure_sbc"){
            return new ServiceBusConnector(pipeid, mode, config);
        }else if(conn_type == "slack"){
            return new SlackConnector(pipeid, mode, config);
        }else if(conn_type == "generator"){
            return new GeneratorConnector(pipeid, mode, config);
        }else{
            throw std::invalid_argument(fmt::format("Unknown Connector type [ {} ]", conn_type));
        }
    }
};


#endif  // __M2E_BRIDGE_CONNECTOR_FACTORY__