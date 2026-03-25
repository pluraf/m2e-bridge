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


#ifndef __M2E_BRIDGE_SCHEMA_H__
#define __M2E_BRIDGE_SCHEMA_H__


#include "m2e_aliases.h"

#include "filtras/all.h"
#include "connectors/all.h"


static json get_schemas(){
    return {
        {"connector", {
#ifdef WITH_MQTT_CONNECTOR
            MqttConnector::get_schema(),
#endif
#ifdef WITH_GCP_PUBSUB_CONNECTOR
            PubSubConnector::get_schema(),
#endif
#ifdef WITH_EMAIL_CONNECTOR
            EmailConnector::get_schema(),
#endif
#ifdef WITH_GCP_CLOUD_STORAGE_CONNECTOR
            CloudStorageConnector::get_schema(),
#endif
#ifdef WITH_AWS_S3_CONNECTOR
            S3Connector::get_schema(),
#endif
#ifdef WITH_HTTP_CONNECTOR
            HttpConnector::get_schema(),
#endif
#ifdef WITH_AZURE_SERVICE_BUS_CONNECTOR
            ServiceBusConnector::get_schema(),
#endif
#ifdef WITH_SLACK_CONNECTOR
            SlackConnector::get_schema(),
#endif
#ifdef WITH_MODBUS_CONNECTOR
            ModbusConnector::get_schema(),
#endif
            GeneratorConnector::get_schema(),
            InternalConnector::get_schema()
        }},
        {"filtra", {
#ifdef WITH_LUA
            LuaConverterFT::get_schema(),
#endif
            ComparatorFT::get_schema(),
            FinderFT::get_schema(),
            EraserFT::get_schema(),
            BuilderFT::get_schema(),
            SplitterFT::get_schema(),
            LimiterFT::get_schema(),
            NopFT::get_schema(),
            ThrottleFT::get_schema()
        }}
    };
}


static json get_schema_by_type(const std::string& type){
    json pipeline_components = get_schemas();
    if(pipeline_components["connectors"].contains(type)){
        return pipeline_components["conectors"][type];
    }else if(pipeline_components["filtras"].contains(type)){
        return pipeline_components["filtras"][type];
    }else{
        throw std::runtime_error("Unknown type");
    }
}


#endif  // __M2E_BRIDGE_SCHEMA_H__