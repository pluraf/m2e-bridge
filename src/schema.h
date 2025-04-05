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

#include "filtras/builder.h"
#include "filtras/comparator.h"
#include "filtras/eraser.h"
#include "filtras/finder.h"
#include "filtras/limiter.h"
#include "filtras/nop.h"
#include "filtras/splitter.h"
#include "filtras/throttle.h"

#include "connectors/all.h"


static json get_schemas(){
    return {
        {"connector", {
            MqttConnector::get_schema(),
            PubSubConnector::get_schema(),
            EmailConnector::get_schema(),
            InternalConnector::get_schema(),
            CloudStorageConnector::get_schema(),
            S3Connector::get_schema(),
            HttpConnector::get_schema(),
            ServiceBusConnector::get_schema(),
            SlackConnector::get_schema(),
            GeneratorConnector::get_schema()

        }},
        {"filtra", {
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