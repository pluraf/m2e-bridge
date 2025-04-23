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


#ifndef __M2E_BRIDGE_DATABASE_CONVERTER_H__
#define __M2E_BRIDGE_DATABASE_CONVERTER_H__


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdexcept>

#include "global_config.h"


struct Converter {
    std::string id;
    std::string code;
    std::string description;
};


class ConverterTable {
    sqlite3 * db_;
    std::string db_path_;
public:
    ConverterTable() {
        db_path_ = gc.get_converters_db_path();
        db_ = nullptr;
    }

    bool get(std::string const & converter_id, Converter & converter) {
        open_db(SQLITE_OPEN_READONLY);
        sqlite3_stmt* stmt;
        int res;
        const char * sql =
            R"(SELECT id, code, description FROM converters WHERE id = ?;)";

        res = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
            close_db();
            return false;
        }

        // Bind the ID
        res = sqlite3_bind_text(stmt, 1, converter_id.c_str(), converter_id.size(), nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db_) << std::endl;
            close_db();
            return false;
        }
        // Execute the query
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            converter.id = get_string(sqlite3_column_text(stmt, 0));
            converter.code = get_string(sqlite3_column_text(stmt, 1));
            converter.description = get_string(sqlite3_column_text(stmt, 2));
        }else {
            std::cerr << fmt::format("Converter [ {} ] not found.", converter_id) << std::endl;
            sqlite3_finalize(stmt);
            close_db();
            return false;
        }

        // Cleanup
        sqlite3_finalize(stmt);
        close_db();
        return true;
    }

private:
    void close_db(){
        if(db_ == nullptr) return;
        int res = sqlite3_close(db_);
        if (res != SQLITE_OK) {
            std::cerr << "Can't close database: " << sqlite3_errmsg(db_) << std::endl;
            throw std::runtime_error("Can't close database");
        }
    }

    void open_db(int flags){
        int res = sqlite3_open_v2(db_path_.c_str(), & db_, flags, nullptr);
        if (res != SQLITE_OK) {
            throw std::runtime_error("Can't open database");
        }
    }

    std::string get_string(unsigned char const * val){
        return (val == nullptr)? std::string() :
            std::string(reinterpret_cast<const char*>(val));
    }
};


#endif  // __M2E_BRIDGE_DATABASE_CONVERTER_H__