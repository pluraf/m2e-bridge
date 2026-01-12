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


#ifndef __M2E_BRIDGE_SQLITE_CONNECTOR_H__
#define __M2E_BRIDGE_SQLITE_CONNECTOR_H__


#include <sstream>
#include <memory>
#include <span>
#include <stdexcept>

#include <sqlite3.h>

#include "connector.h"
#include "substitutions/subs.hpp"


struct SQLiteConnectorConfig
{
    string db_path;
    string table;
    vector<string> columns;
    vector<string> values;
};


class SQLiteConnector: public Connector
{
    SQLiteConnectorConfig config_;
    sqlite3 *db_;
    string statement_;

public:
    SQLiteConnector(std::string pipeid, ConnectorMode mode, json const & config)
        :Connector(pipeid, mode, config)
    {
        config_.db_path = config.at("db_path").get<string>();
        config_.table = config.at("table").get<string>();

        auto j_columns {config.at("columns")};
        config_.columns = vector<string>(j_columns.begin(), j_columns.end());

        auto j_values {config.at("values")};
        config_.values = vector<string>(j_values.begin(), j_values.end());
    }

    void do_connect() override
    {
        statement_ = build_statement();
    }

    void do_send(MessageWrapper & msg_w) override
    {
        int res = sqlite3_open_v2(config_.db_path.c_str(), &db_, SQLITE_OPEN_READWRITE, nullptr);
        if(res != SQLITE_OK){ throw std::runtime_error("Can't open database"); }

        sqlite3_stmt *stmt;
        res = sqlite3_prepare_v2(db_, statement_.c_str(), -1, &stmt, nullptr);
        if(res != SQLITE_OK){
            sqlite3_close(db_);
            throw std::runtime_error(sqlite3_errmsg(db_));
        }

        auto se = SubsEngine(msg_w);

        // Build a query and do substitutions
        unsigned pix {0};
        // Make sure vector does not reallocate elements as we use .c_str()
        vector<substituted_t> row_values(config_.values.size());

        for(auto &vtemplate : config_.values){
            row_values.push_back(se.substitute(vtemplate));
            auto &v = row_values.back();

            if( std::holds_alternative<std::vector<unsigned char>>(v) )
            {
                auto & d {std::get<std::vector<unsigned char>>(v)};
                sqlite3_bind_blob(stmt, ++pix, d.data(), d.size(), nullptr);
            }
            else if( std::holds_alternative<std::span<std::byte const>>(v) )
            {
                auto d {std::get<std::span<std::byte const>>(v)};
                sqlite3_bind_blob(stmt, ++pix, d.data(), d.size(), nullptr);
            }
            else if( std::holds_alternative<string>(v) )
            {
                sqlite3_bind_text(stmt, ++pix, std::get<string>(v).c_str(), -1, nullptr);
            }
            else if( std::holds_alternative<double>(v) )
            {
                sqlite3_bind_double(stmt, ++pix, std::get<double>(v));
            }
            else if( std::holds_alternative<long>(v) )
            {
                sqlite3_bind_int64(stmt, ++pix, std::get<long>(v));
            }
            else if( std::holds_alternative<bool>(v) )
            {
                sqlite3_bind_int64(stmt, ++pix, std::get<bool>(v));
            }
            else{
                throw std::runtime_error("Unexpected substituted value!");
            }
        }

        res = sqlite3_step(stmt);
        if (res != SQLITE_DONE) { throw std::runtime_error(sqlite3_errmsg(db_)); }

        res = sqlite3_finalize(stmt);
        if (res != SQLITE_OK) { throw std::runtime_error(sqlite3_errmsg(db_)); }

        res = sqlite3_close(db_);
        if (res != SQLITE_OK) { throw std::runtime_error(sqlite3_errmsg(db_)); }
    }

    string build_statement()
    {
        std::ostringstream into;
        std::ostringstream values;

        into << "INSERT INTO " << config_.table << "(";
        values << " VALUES(";

        auto it = config_.columns.begin();
        auto end = config_.columns.end();

        if(it != end){
            into << *it++;
            values << "?";
        }
        while(it != end){
            into << ", " << *it;
            ++it;
            values << ", ?";
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
        return {"sqlite", schema};
    }
};


#endif  // __M2E_BRIDGE_SQLITE_CONNECTOR_H__
