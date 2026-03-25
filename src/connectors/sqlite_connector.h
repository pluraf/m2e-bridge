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


//_DOCS: SECTION_START sqlite_connector SQLite Connector
/*!
.. _SQLite: https://sqlite.org

Connects to a `SQLite`_ database::

    {
        "type": "sqlite",
        "db_path": "<path>",
        "table": "<table>",
        "columns": [
            "<column1>",
            "<colimn2>"
        ],
        "values": [
            "<value1>",
            "<value1>"
        ]
    }
*/
//_DOCS: END

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
        if( res != SQLITE_OK )
        {
            throw std::runtime_error("Can't open database: " + config_.db_path);
        }

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

    static pair<string, json> get_schema()
    {
        //_DOCS: SCHEMA_START sqlite_connector
        //_DOCS: SCHEMA_INCLUDE connector
        static json schema = Connector::get_schema(
            {
                {"tags", {"database", "sqlite"}},
                {"modes", {"out"}},
                {"type_properties", {
                    {"db_path", {
                        {"type", "string"},
                        {"required", true},
                        {"description", "Database file."}
                    }},
                    {"table", {
                        {"type", "string"},
                        {"required", true},
                        {"description", "Database table."}
                    }},
                    {"columns", {
                        {"type", "array"},
                        {"required", true},
                        {"description", "Columns where values will be inserted."}
                    }},
                    {"values", {
                        {"type", "array"},
                        {"required", true},
                        {"description", "Values to be inserted into the table."}
                    }}
                }}
            }
        );
        //_DOCS: END
        return {"sqlite", schema};
    }
};


#endif  // __M2E_BRIDGE_SQLITE_CONNECTOR_H__
