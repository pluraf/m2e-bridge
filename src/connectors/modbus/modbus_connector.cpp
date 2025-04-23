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


#include <grpcpp/grpcpp.h>
#include <fmt/core.h>

#include "modbus_connector.h"


using agent_api_modbus::ModbusAgent;
using agent_api_modbus::ReadRequest;
using agent_api_modbus::ReadBitsResponse;
using agent_api_modbus::ReadRegistersResponse;
using agent_api_modbus::WriteBitsRequest;
using agent_api_modbus::WriteRegistersRequest;
using agent_api_modbus::WriteResponse;
using agent_api_modbus::ConnectRequest;
using agent_api_modbus::ConnectResponse;
using agent_api_modbus::DisconnectRequest;
using agent_api_modbus::DisconnectResponse;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;


template<>
ModbusTable lexical_cast<ModbusTable>(string const & s)
{
    if (s == "coils" || s == "COILS") return ModbusTable::COILS;
    if (s == "discrete_inputs" || s == "DISCRETE_INPUTS") return ModbusTable::DISCRETE_INPUTS;
    if (s == "input_registers" || s == "INPUT_REGISTERS") return ModbusTable::INPUT_REGISTERS;
    if (s == "holding_registers" || s == "HOLDING_REGISTERS") return ModbusTable::HOLDING_REGISTERS;
    return ModbusTable::UNKN;
}

template<>
string lexical_cast<ModbusTable>(ModbusTable const & mt)
{
    if (mt == ModbusTable::COILS) return "coils";
    if (mt == ModbusTable::DISCRETE_INPUTS) return "discrete_inputs";
    if (mt == ModbusTable::INPUT_REGISTERS) return "input_registers";
    if (mt == ModbusTable::HOLDING_REGISTERS) return "holding_registers";
    return "unkn";
}


ModbusConnector::ModbusConnector(std::string pipeid, ConnectorMode mode, json const & config):
    Connector(pipeid, mode, config)
{
    // modbus agent
    try{
        agent_ = config.at("agent").get<string>();
    }catch(json::exception){
        agent_ = "127.0.0.1:1502";
    }
    // modbus gateway
    gateway_ = config.at("gateway").get<string>();
    // polling period
    if(polling_period_ == 0){
        throw std::runtime_error("polling_period_ must be larger than 0!");
    }
    // table
    table_ = lexical_cast<ModbusTable>(config.at("table").get<string>());
    // device address
    device_address_ = config.at("device_address").get<uint8_t>();
    // data address
    data_address_ = config.at("data_address").get<uint16_t>();
    // quantity
    try{
        quantity_ = config.at("quantity").get<uint16_t>();
    }catch(json::exception){
        quantity_ = 1;
    }

    stub_ = ModbusAgent::NewStub(
        grpc::CreateChannel(agent_, grpc::InsecureChannelCredentials())
    );
}


void ModbusConnector::connect()
{
    ConnectRequest request;
    ConnectResponse response;
    ClientContext context;
    Status status;

    request.set_server(gateway_);
    status = stub_->Connect(& context, request, & response);
    if(status.ok()){
        connection_id_ = response.status();
    }else{
        throw std::runtime_error("MODBUS Connector: Connection Error");
    }
}


void ModbusConnector::disconnect()
{
    DisconnectRequest request;
    DisconnectResponse response;
    ClientContext context;
    Status status;

    request.set_connection_id(connection_id_);
    status = stub_->Disconnect(& context, request, & response);
    if(status.ok()){
        connection_id_ = 0;
    }else{
        throw std::runtime_error("MODBUS Connector: Disconnection Error");
    }
}


Message const ModbusConnector::do_receive()
{
    ReadRequest request;
    ReadBitsResponse response_bits;
    ReadRegistersResponse response_registers;
    ClientContext context;
    Status status;

    request.set_connection_id(connection_id_);
    request.set_device_address(device_address_);
    request.set_data_address(data_address_);
    request.set_quantity(quantity_);

    if(table_ == ModbusTable::COILS){
        status = stub_->ReadCoils(& context, request, & response_bits);
    }else if(table_ == ModbusTable::DISCRETE_INPUTS){
        status = stub_->ReadDiscreteInputs(& context, request, & response_bits);
    }else if(table_ == ModbusTable::INPUT_REGISTERS){
        status = stub_->ReadInputRegisters(& context, request, & response_registers);
    }else if(table_ == ModbusTable::HOLDING_REGISTERS){
        status = stub_->ReadHoldingRegisters(& context, request, & response_registers);
    }else{
        throw std::runtime_error("Unknown MODBUS Table!");
    }

    if(status.ok()){
        if(table_ == ModbusTable::COILS || table_ == ModbusTable::DISCRETE_INPUTS){
            if(response_bits.status() == -1){
                throw std::runtime_error(
                    fmt::format("MODBUS Error Number: {}", response_bits.error())
                );
            }
            std::vector<std::byte> ret;
            for(int ix = 0; ix < response_bits.bits_size(); ++ ix){
                ret.push_back(static_cast<std::byte>(response_bits.bits(ix)));
            }
            return Message(ret);
        }else if(table_ == ModbusTable::INPUT_REGISTERS || table_ == ModbusTable::HOLDING_REGISTERS){
            if(response_registers.status() == -1){
                throw std::runtime_error(
                    fmt::format("MODBUS Error Number: {}", response_registers.error())
                );
            }
            std::vector<std::byte> ret;
            for(int ix = 0; ix < response_registers.registers_size(); ++ ix){
                uint16_t reg = response_registers.registers(ix);
                ret.push_back(static_cast<std::byte>(reg >> 8 & 0xFF));
                ret.push_back(static_cast<std::byte>(reg & 0xFF));
            }
            return Message(ret);
        }
    }
    throw std::runtime_error("Unknown MODBUS Connector Error");
}


void ModbusConnector::do_send(MessageWrapper & msg_w)
{

}
