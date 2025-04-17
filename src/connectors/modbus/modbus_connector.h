/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2025 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_BRIDGE_MODBUS_CONNECTOR_H__
#define __M2E_BRIDGE_MODBUS_CONNECTOR_H__


#include "../connector.h"

#include "agent_api_modbus.grpc.pb.h"


enum class ModbusTable{
    UNKN,
    COILS,
    DISCRETE_INPUTS,
    INPUT_REGISTERS,
    HOLDING_REGISTERS
};


class ModbusConnector: public Connector{
    string agent_;
    string gateway_;
    ModbusTable table_;
    uint8_t device_address_ {};
    uint16_t data_address_ {};
    uint16_t quantity_ {};
    std::unique_ptr<agent_api_modbus::ModbusAgent::Stub> stub_;
    size_t connection_id_ {};
public:
    ModbusConnector(std::string pipeid, ConnectorMode mode, json const & config);
    void connect()override;
    void disconnect()override;

    static pair<string, json> get_schema(){
        json schema = Connector::get_schema();
        schema.merge_patch({
            {"authbundle_id", {
                {"options", {
                    {"filter", {
                        {"key", "service_type"},
                        {"value", "modbus"}
                    }}
                }}
            }},
            {"polling_period", {
                {"type", "integer"},
                {"required", true}
            }},
            {"server", {
                {"type", "string"},
                {"server", "127.0.0.1:1502"},
                {"required", false}
            }}
        });
        return {"http", schema};
    }
protected:
    void do_send(MessageWrapper & msg_w)override;
    Message const do_receive()override;
};


#endif  // __M2E_BRIDGE_MODBUS_CONNECTOR_H__