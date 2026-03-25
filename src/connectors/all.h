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


#ifndef __M2E_BRIDGE_ALL_CONNECTORS_H__
#define __M2E_BRIDGE_ALL_CONNECTORS_H__


#include "connectors/internal_connector.h"
#include "connectors/generator_connector.h"


#ifdef WITH_EMAIL_CONNECTOR
    #include "connectors/email_connector.h"
#endif

#ifdef WITH_HTTP_CONNECTOR
    #include "connectors/http_connector.h"
#endif

#ifdef WITH_AWS_S3_CONNECTOR
    #include "connectors/aws_s3_connector.h"
#endif

#ifdef WITH_AZURE_SERVICE_BUS_CONNECTOR
    #include "connectors/azure_service_bus_connector.h"
#endif

#ifdef WITH_GCP_CLOUD_STORAGE_CONNECTOR
    #include "connectors/gcp_cloud_storage_connector.h"
#endif

#ifdef WITH_GCP_PUBSUB_CONNECTOR
    #include "connectors/gcp_pubsub_connector.h"
#endif

#ifdef WITH_MQTT_CONNECTOR
    #include "connectors/mqtt_connector.h"
#endif

#ifdef WITH_SLACK_CONNECTOR
    #include "connectors/slack_connector.h"
#endif

#ifdef WITH_MODBUS_CONNECTOR
    #include "connectors/modbus/modbus_connector.h"
#endif

#ifdef WITH_POSTGRESQL_CONNECTOR
    #include "connectors/postgresql_connector.h"
#endif

#ifdef WITH_SQLITE_CONNECTOR
    #include "connectors/sqlite_connector.h"
#endif


#endif  // __M2E_BRIDGE_ALL_CONNECTORS_H__