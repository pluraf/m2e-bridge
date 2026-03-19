#ifndef __TEST_MOCK_DATABASE_H__
#define __TEST_MOCK_DATABASE_H__


#include <sqlite3.h>

#include  "../src/connectors/email_connector.h"
#include "../src/global_config.h"


void create_test_database(std::string config_path);


#endif  // __TEST_MOCK_DATABASE_H__