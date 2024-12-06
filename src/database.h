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


#ifndef __M2E_BRIDGE_DATABASE_H__
#define __M2E_BRIDGE_DATABASE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdexcept>

#include "global_config.h"

enum class ServiceType{
    GCP,
    MQTT311,
    MQTT50,
    EMAIL,
    AWS,
    AZURE,
    NONE
};

ServiceType get_service_type(string const & val){
    if(val == "gcp") return ServiceType::GCP;
    if(val == "mqtt311") return ServiceType::MQTT311;
    if(val == "mqtt50") return ServiceType::MQTT50;
    if(val == "email") return ServiceType::EMAIL;
    if(val == "aws") return ServiceType::AWS;
    if(val == "azure") return ServiceType::AZURE;
    return ServiceType::NONE;
}

std::string service_type_to_string(ServiceType ct){
     switch (ct) {
        case ServiceType::GCP: return "gcp";
        case ServiceType::MQTT311: return "mqtt311";
        case ServiceType::MQTT50: return "mqtt50";
        case ServiceType::EMAIL: return "email";
        case ServiceType::AWS: return "aws";
        case ServiceType::AZURE: return "azure";
        case ServiceType::NONE: return "";
        default: return "";
    }
}

enum class AuthType{
    JWT_ES256,
    PASSWORD,
    SERVICE_KEY,
    ACCESS_KEY,
    NONE
};

AuthType get_auth_type(std::string val){
    if(val == "jwt_es256")
        return AuthType::JWT_ES256;
    else if(val == "password")
        return AuthType::PASSWORD;
    else if(val == "service_key")
        return AuthType::SERVICE_KEY;
    else if(val == "access_key")
        return AuthType::ACCESS_KEY;
    else
        return AuthType::NONE;
}

std::string auth_type_to_string(AuthType at){
    switch (at) {
        case AuthType::JWT_ES256: return "jwt_es256";
        case AuthType::PASSWORD: return "password";
        case AuthType::SERVICE_KEY: return "service_key";
        case AuthType::ACCESS_KEY: return "access_key";
        case AuthType::NONE: return "";
        default: return "";
    }
}

struct AuthBundle {
    std::string authbundle_id;
    ServiceType service_type;
    AuthType auth_type;
    std::string username;
    std::string password;
    std::string keyname;
    std::string keydata;
    std::string description;
};

void print_authbundle(const AuthBundle& bundle) {
    std::cout << "AuthBundle ID: " << bundle.authbundle_id << std::endl;
    std::cout << "Service Type: " << service_type_to_string(bundle.service_type) << std::endl;
    std::cout << "Auth Type: " << auth_type_to_string(bundle.auth_type) << std::endl;
    std::cout << "Username: " << bundle.username << std::endl;
    std::cout << "Password: " << bundle.password << std::endl;
    std::cout << "Keyname: " << bundle.keyname << std::endl;
    std::cout << "Keydata: " << bundle.keydata << std::endl;
    std::cout << "Description: " << bundle.description << std::endl;
}

std::string get_string(const unsigned char * val){
    return (val == nullptr)? std::string() :
        std::string(reinterpret_cast<const char*>(val));
}

class Database {
public:
    Database() {
        db_path_ = gc.get_authbundles_db_path();
        db_ = nullptr;
    }

    bool insert_authbundle(const AuthBundle& authbundle) {
        open_db(SQLITE_OPEN_READWRITE);
        const char* sql = R"(INSERT INTO authbundles (authbundle_id, service_type, auth_type, username, password, keyname, description, keydata)
                             VALUES (?, ?, ?, ?, ?, ?, ?, ?);)";
        sqlite3_stmt* stmt;
        int res = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
            close_db();
            return false;
        }

        std::string service_type_str = service_type_to_string(authbundle.service_type);
        std::string auth_type_str = auth_type_to_string(authbundle.auth_type);

        sqlite3_bind_text(stmt, 1, authbundle.authbundle_id.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 2, service_type_str.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 3, auth_type_str.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 4, authbundle.username.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 5, authbundle.password.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 6, authbundle.keyname.c_str(), -1, nullptr);
        sqlite3_bind_text(stmt, 7, authbundle.description.c_str(), -1, nullptr);
        sqlite3_bind_blob(stmt, 8, authbundle.keydata.data(), authbundle.keydata.size(), nullptr);

        res = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        close_db();
        return res == SQLITE_DONE;
    }

    bool delete_authbundle(const std::string& authbundle_id) {
        open_db(SQLITE_OPEN_READWRITE);
        const char* sql = "DELETE FROM authbundles WHERE authbundle_id = ?;";
        sqlite3_stmt* stmt;
        int res = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
            close_db();
            return false;
        }
        sqlite3_bind_text(stmt, 1, authbundle_id.c_str(), -1, nullptr);

        res = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        close_db();
        return res == SQLITE_DONE;
    }

    bool retrieve_authbundle(std::string authbundle_id, AuthBundle & authbundle) {
        open_db(SQLITE_OPEN_READONLY);
        sqlite3_stmt* stmt;
        int res;
        const char * sql = R"(SELECT authbundle_id, service_type,
                    auth_type, username, password,
                    keyname, description, keydata
                    FROM authbundles WHERE authbundle_id = ?;)";

        res = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_) << std::endl;
            close_db();
            return false;
        }

        // Bind the ID
        res = sqlite3_bind_text(stmt, 1, authbundle_id.c_str(), authbundle_id.size(), nullptr);
        if (res != SQLITE_OK) {
            std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db_) << std::endl;
            close_db();
            return false;
        }
        // Execute the query
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            authbundle.authbundle_id = get_string(sqlite3_column_text(stmt, 0));
            authbundle.service_type = get_service_type(get_string(sqlite3_column_text(stmt, 1)));
            authbundle.auth_type = get_auth_type(get_string(sqlite3_column_text(stmt, 2)));
            authbundle.username = get_string(sqlite3_column_text(stmt, 3));
            authbundle.password = get_string(sqlite3_column_text(stmt, 4));
            authbundle.keyname = get_string(sqlite3_column_text(stmt, 5));
            authbundle.description = get_string(sqlite3_column_text(stmt, 6));

            // Retrieve the BLOB data
            const void* blobData = sqlite3_column_blob(stmt, 7);
            int blobSize = sqlite3_column_bytes(stmt, 7);
            if (blobData && blobSize > 0) {
                authbundle.keydata = std::string(static_cast<const char*>(blobData), blobSize);
            }
        }else {
            std::cerr << fmt::format("Authbundle [ {} ] not found.", authbundle_id) << std::endl;
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
    sqlite3* db_;
    std::string db_path_;

    void close_db(){
        if(db_ == nullptr) return;
        int res = sqlite3_close(db_);
        if (res != SQLITE_OK) {
            std::cerr << "Can't close database: " << sqlite3_errmsg(db_) << std::endl;
            throw std::runtime_error("Can't close database");
        }
    }

    void open_db(int flags){
        int res = sqlite3_open_v2(db_path_.c_str(), &db_, flags, nullptr);
        if (res != SQLITE_OK) {
            throw std::runtime_error("Can't open database");
        }
    }


};


#endif  // __M2E_BRIDGE_DATABASE_H__