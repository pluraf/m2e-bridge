/* SPDX-License-Identifier: BSD-3-Clause */

/*
Copyright (c) 2024-2025 Pluraf Embedded AB <code@pluraf.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
        }else if(conn_type == "modbus"){
            return new ModbusConnector(pipeid, mode, config);
        }
        else if(conn_type == "postgresql"){
            return new PostgreSQLConnector(pipeid, mode, config);
        }
        else{
            throw std::invalid_argument(fmt::format("Unknown Connector type [ {} ]", conn_type));
        }
    }
};


#endif  // __M2E_BRIDGE_CONNECTOR_FACTORY__