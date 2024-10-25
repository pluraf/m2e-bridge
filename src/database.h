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

enum class ConnectorType{
    GCP_PUBSUB,
    GCP_BUCKET,
    MQTT311,
    MQTT50,
    EMAIL,
    NONE
};

ConnectorType get_connector_type(std::string val){
    if(val == "gcp_pubsub")
        return ConnectorType::GCP_PUBSUB;
    else if(val == "gcp_bucket")
        return ConnectorType::GCP_BUCKET;
    else if(val == "mqtt311")
        return ConnectorType::MQTT311;
    else if(val == "mqtt50")
        return ConnectorType::MQTT50;
    else if(val == "email")
        return ConnectorType::EMAIL;
    else
        return ConnectorType::NONE;
}

std::string connector_type_to_string(ConnectorType ct){
     switch (ct) {
        case ConnectorType::GCP_PUBSUB: return "gcp_pubsub";
        case ConnectorType::GCP_BUCKET: return "gcp_bucket";
        case ConnectorType::MQTT311: return "mqtt311";
        case ConnectorType::MQTT50: return "mqtt50";
        case ConnectorType::EMAIL: return "email";
        case ConnectorType::NONE: return "";
        default: return "";
    }
}

enum class AuthType{
    JWT_ES256,
    PASSWORD,
    SERVICE_KEY,
    NONE
};

AuthType get_auth_type(std::string val){
    if(val == "jwt_es256")
        return AuthType::JWT_ES256;
    else if(val == "password")
        return AuthType::PASSWORD;
    else if(val == "service_key")
        return AuthType::SERVICE_KEY;
    else
        return AuthType::NONE;
}

std::string auth_type_to_string(AuthType at){
    switch (at) {
        case AuthType::JWT_ES256: return "jwt_es256";
        case AuthType::PASSWORD: return "password";
        case AuthType::SERVICE_KEY: return "service_key";
        case AuthType::NONE: return "";
        default: return "";
    }
}

struct AuthBundle {
    std::string authbundle_id;
    ConnectorType connector_type;
    AuthType auth_type;
    std::string username;
    std::string password;
    std::string keyname;
    std::string keydata;
    std::string description;
};

void print_authbundle(const AuthBundle& bundle) {
    std::cout << "AuthBundle ID: " << bundle.authbundle_id << std::endl;
    std::cout << "Connector Type: " << connector_type_to_string(bundle.connector_type) << std::endl;
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

    bool retrieve_authbundle(std::string authbundle_id, AuthBundle& authbundle) {
        open_db(SQLITE_OPEN_READONLY);
        sqlite3_stmt* stmt;
        int res;
        const char * sql = R"(SELECT authbundle_id, connector_type,
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
            authbundle.connector_type = get_connector_type(get_string(sqlite3_column_text(stmt, 1)));
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