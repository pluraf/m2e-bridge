/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024-2026 Pluraf Embedded AB <code@pluraf.com>

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

#include "connectors/all.h"


class ConnectorFactory{
public:
    static Connector * create(
            std::string const & pipeid, ConnectorMode mode, json const & config){
        string const & conn_type = config.at("type");

        if( conn_type == "queue" )
        {
            return new InternalConnector(pipeid, mode, config);
        }
        else if(conn_type == "generator")
        {
            return new GeneratorConnector(pipeid, mode, config);
        }
#ifdef WITH_MQTT_CONNECTOR
        else if( conn_type == "mqtt" )
        {
            return new MqttConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_GCP_PUBSUB_CONNECTOR
        else if( conn_type == "gcp_pubsub" )
        {
            return new PubSubConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_GCP_CLOUD_STORAGE_CONNECTOR
        else if( conn_type == "gcp_cloud_storage" )
        {
            return new CloudStorageConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_EMAIL_CONNECTOR
        else if( conn_type == "email" )
        {
            return new EmailConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_AWS_S3_CONNECTOR
        else if(conn_type == "aws_s3")
        {
            AwsSdkManager::Instance();
            return new S3Connector(pipeid, mode, config);
        }
#endif
#ifdef WITH_HTTP_CONNECTOR
        else if( conn_type == "http" )
        {
            return new HttpConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_AZURE_SERVICE_BUS_CONNECTOR
        else if(conn_type == "azure_sbc")
        {
            return new ServiceBusConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_SLACK_CONNECTOR
        else if(conn_type == "slack")
        {
            return new SlackConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_MODBUS_CONNECTOR
        else if(conn_type == "modbus")
        {
            return new ModbusConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_POSTGRESQL_CONNECTOR
        else if(conn_type == "postgresql")
        {
            return new PostgreSQLConnector(pipeid, mode, config);
        }
#endif
#ifdef WITH_SQLITE_CONNECTOR
        else if(conn_type == "sqlite")
        {
            return new SQLiteConnector(pipeid, mode, config);
        }
#endif
        else
        {
            throw std::invalid_argument(fmt::format("Unknown Connector type [ {} ]", conn_type));
        }
    }
};


#endif  // __M2E_BRIDGE_CONNECTOR_FACTORY__