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


using channel_modbus_api::ModbusChannel;
using channel_modbus_api::ReadRegistersResponse;
using channel_modbus_api::ReadBitsResponse;
using channel_modbus_api::ReadRequest;
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
    // server
    try{
        server_ = config.at("server").get<string>();
    }catch(json::exception){
        server_ = "127.0.0.1:1502";
    }
    // table
    table_ = lexical_cast<ModbusTable>(config.at("table").get<string>());
    // address
    address_ = config.at("address").get<unsigned>();
    // quantity
    try{
        quantity_ = config.at("quantity").get<unsigned>();
    }catch(json::exception){
        quantity_ = 1;
    }

    stub_ = ModbusChannel::NewStub(
        grpc::CreateChannel(server_, grpc::InsecureChannelCredentials())
    );
}


Message const ModbusConnector::do_receive()
{
    ReadRequest request;
    ReadBitsResponse response_bits;
    ReadRegistersResponse response_registers;
    ClientContext context;
    Status status;

    request.set_address(address_);
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

    if (status.ok()){
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
