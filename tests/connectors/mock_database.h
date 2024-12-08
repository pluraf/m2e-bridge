#ifndef TEST_DATABASE_H
#define TEST_DATABASE_H

#include <sqlite3.h>

#include  "../src/connectors/email_connector.h"

#include "../src/database.h"
#include "../src/global_config.h"


GlobalConfig gc;

void create_test_database(std::string config_path){
    gc.load(config_path);
    std::string db_path = gc.get_authbundles_db_path();
    sqlite3* db_;
    int res = sqlite3_open(db_path.c_str(), &db_);
    if (res != SQLITE_OK) {
        std::cerr << "Failed to open test database: " << sqlite3_errmsg(db_) << std::endl;
        return;
    }
    const char* create_table_sql = R"(
        CREATE TABLE IF NOT EXISTS authbundles (
            authbundle_id TEXT PRIMARY KEY,
            service_type TEXT,
            auth_type TEXT,
            username TEXT,
            password TEXT,
            keyname TEXT,
            description TEXT,
            keydata BLOB
        );
    )";

    char* err_msg = nullptr;
    res = sqlite3_exec(db_, create_table_sql, nullptr, nullptr, &err_msg);
    if (res != SQLITE_OK) {
        std::cerr << "Failed to create table: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        sqlite3_close(db_);
        throw std::runtime_error("Failed to create table in test database");
    }
    sqlite3_close(db_);
}


#endif