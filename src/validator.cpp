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


pair<bool, string> validate_connector(ConnectorMode mode, json const & config){
    try{
        string const & conn_type = config.at("type");
        if(conn_type == "mqtt"){
            MqttConnector("pipeid", mode, config);
        }else if(conn_type == "gcp_pubsub"){
            PubSubConnector("pipeid", mode, config);
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
        }else if(conn_type == "generator"){
            GeneratorConnector("pipeid", mode, config);
        }else if(conn_type == "modbus"){
            ModbusConnector("pipeid", mode, config);
        }else{
            return {false, "Unknown connector type: " + conn_type + "!"};
        }
        return {true, ""};
    }catch(std::out_of_range const & e){
        return {false, e.what()};
    }catch(json::exception const & e){
        return {false, e.what()};
    }catch(std::runtime_error const & e){
        return {true, ""};
    }
}


pair<bool, string> validate_filtra(json const & config){
    return {true, ""};
}


pair<bool, string> validate_pipeline(json const & config){
    if(! config.contains("connector_in")) return {false, "Missing connector_in"};
    if(! config.contains("connector_out")) return {false, "Missing connector_out"};

    // Validate connectors
    auto res = validate_connector(ConnectorMode::IN, config["connector_in"]);
    if(! res.first) return res;

    res = validate_connector(ConnectorMode::OUT, config["connector_out"]);
    return res;
}