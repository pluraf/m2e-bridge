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

#include "connectors/aws_s3_connector.h"
#include "connectors/azure_service_bus_connector.h"
#include "connectors/email_connector.h"
#include "connectors/gcp_bucket_connector.h"
#include "connectors/gcp_pubsub_connector.h"
#include "connectors/http_connector.h"
#include "connectors/internal_connector.h"
#include "connectors/mqtt_connector.h"
#include "connectors/slack_connector.h"


static json get_schemas(){
    return {
        {"connectors", {
            mqtt_connector_schema_,
            gcp_pubsub_connector_schema_,
            email_connector_schema_,
            internal_connector_schema_,
            gcp_bucket_connector_schema_,
            aws_s3_connector_schema_,
            http_connector_schema_,
            service_bus_connector_schema_,
            slack_connector_schema_
        }},
        {"filtras", {
            comparator_filtra_schema_,
            finder_filtra_schema_,
            eraser_filtra_schema_,
            builder_filtra_schema_,
            splitter_filtra_schema_,
            limiter_filtra_schema_,
            nop_filtra_schema_,
            throttle_filtra_schema_
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