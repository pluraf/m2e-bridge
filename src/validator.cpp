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


#include <stdexcept>

#include "validator.h"
#include "connectors/all.h"


bool validate_connector(ConnectorMode mode, json const & config){
    try{
        string const & conn_type = config.at("type");
        if(conn_type == "mqtt"){
            MqttConnector("pipeid", mode, config);
        }else if(conn_type == "gcp_pubsub"){
            gcp::PubSubConnector("pipeid", mode, config);
        }else if(conn_type == "email"){
            EmailConnector("pipeid", mode, config);
        }else if(conn_type == "queue"){
            InternalConnector("pipeid", mode, config);
        }else if(conn_type == "gcp_bucket"){
            CloudStorageConnector("pipeid", mode, config);
        }else if(conn_type == "aws_s3"){
            S3Connector("pipeid", mode, config);
        }else if(conn_type == "http"){
            HttpConnector("pipeid", mode, config);
        }else if(conn_type == "azure_sbc"){
            ServiceBusConnector("pipeid", mode, config);
        }else if(conn_type == "slack"){
            SlackConnector("pipeid", mode, config);
        }
        return true;
    }catch(std::out_of_range){
        return false;
    }catch(json::exception){
        return false;
    }
}


bool validate_filtra(json const & config){
    return false;
}


bool validate_pipeline(json const & config){
    if(! config.contains("connector_in")
            || ! config["connector_in"].is_object()
            || ! config.contains("connector_out")
            || ! config["connector_out"].is_object()){
        return false;
    }

    // Validate connectors
    if(! validate_connector(ConnectorMode::IN, config["connector_in"])) return false;
    if(! validate_connector(ConnectorMode::OUT, config["connector_out"])) return false;

    return true;
}