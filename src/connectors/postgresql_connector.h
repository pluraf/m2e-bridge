/* SPDX-License-Identifier: BSD-3-Clause */

/*
Copyright (c) 2025 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_BRIDGE_POSTGRESQL_CONNECTOR_H__
#define __M2E_BRIDGE_POSTGRESQL_CONNECTOR_H__


#include <sstream>
#include <memory>
#include <span>

#include <pqxx/pqxx>

#include "connector.h"
#include "database/authbundle.h"
#include "substitutions/subs.hpp"


struct PostgreSQLConnectorConfig
{
    string db_name;
    string db_host;
    unsigned db_port;
    string table;
    vector<string> columns;
    vector<string> values;
};


class PostgreSQLConnector: public Connector
{
    PostgreSQLConnectorConfig config_;
    std::unique_ptr<pqxx::connection> db_conn_;
    string statement_;

public:
    PostgreSQLConnector(std::string pipeid, ConnectorMode mode, json const & config)
        :Connector(pipeid, mode, config)
    {
        config_.db_name = config.at("db_name").get<string>();
        config_.db_host = config.at("db_host").get<string>();
        config_.db_port = config.at("db_port").get<unsigned>();
        config_.table = config.at("table").get<string>();

        auto j_columns {config.at("columns")};
        config_.columns = vector<string>(j_columns.begin(), j_columns.end());

        auto j_values {config.at("values")};
        config_.values = vector<string>(j_values.begin(), j_values.end());
    }

    virtual void do_send(MessageWrapper & msg_w)
    {
        auto se = SubsEngine(msg_w);

        pqxx::params p;
        // Make sure vector does not reallocate elements as we use views
        vector<substituted_t> row_values(config_.values.size());

        for(auto &vtemplate : config_.values){
            row_values.push_back(se.substitute(vtemplate));
            auto &v = row_values.back();

            if( std::holds_alternative<std::vector<unsigned char>>(v) )
            {
                auto & d {std::get<std::vector<unsigned char>>(v)};
                p.append(std::basic_string_view<std::byte>(reinterpret_cast<std::byte *>(d.data()), d.size()));
            }
            else if( std::holds_alternative<std::span<std::byte const>>(v) )
            {
                auto d { std::get<std::span<std::byte const>>(v) };
                p.append( std::basic_string_view<std::byte>(d.data(), d.size()) );
            }
            else if( std::holds_alternative<string>(v) )
            {
                p.append(std::get<string>(v));
            }
            else if( std::holds_alternative<long>(v) )
            {
                p.append(std::get<long>(v));
            }
            else if( std::holds_alternative<double>(v) )
            {
                p.append(std::get<double>(v));
            }
            else{
                throw std::runtime_error("Unexpected substituted value!");
            }
        }

        pqxx::work txn(*db_conn_);
        pqxx::result r = txn.exec(pqxx::prepped{"insert"}, p);
        txn.commit();
    }

    virtual void do_connect()
    {
        AuthbundleTable db;
        AuthBundle ab;
        bool res = db.get(authbundle_id_, ab);
        if(! res){ throw std::runtime_error("Not able to retrieve authbundle!"); }

        string conn_str = fmt::format(
            "dbname={} user={} password={} host={} port={}",
            config_.db_name,
            ab.username,
            ab.password,
            config_.db_host,
            config_.db_port
        );

        db_conn_ = std::make_unique<pqxx::connection>(conn_str);

        db_conn_->prepare("insert", build_statement());
    }

    virtual void do_disconnect()
    {
        if( db_conn_ ){ db_conn_->close(); }
    }

    string build_statement()
    {
        std::ostringstream into;
        std::ostringstream values;

        into << "INSERT INTO " << config_.table << "(";
        values << " VALUES(";

        auto it = config_.columns.begin();
        auto end = config_.columns.end();

        unsigned vcnt {0};
        if(it != end){
            into << *it++;
            values << "$" << ++vcnt;
        }
        while(it != end){
            into << ", " << *it;
            ++it;
            values << ", $" << ++vcnt;
        }

        into << ")";
        values << ")";

        return into.str() + values.str();
    }

    static pair<string, json> get_schema(){
        json schema = Connector::get_schema();
        schema.merge_patch({
           {"authbundle_id", {
                {"options", {
                    {"filter", {
                        {"key", "service_type"},
                        {"value", "database"}
                    }}
                }}
            }},
            {"table", {
                {"type", "string"},
                {"required", true}
            }},
            {"columns", {
                {"type", "array"},
                {"required", true}
            }},
            {"values", {
                {"type", "array"},
                {"required", true}
            }}
        });
        return {"postgresql", schema};
    }
};


#endif  // __M2E_BRIDGE_POSTGRESQL_CONNECTOR_H__
