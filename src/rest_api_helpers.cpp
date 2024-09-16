#include <iostream>
#include <fstream>

#include "rest_api_helpers.h"

bool parse_request_body(struct mg_connection *conn, std::string &pipe_id, json &pipeline_data) {
    char buf[1024]; 
    int length = mg_read(conn, buf, sizeof(buf)); 
    buf[length] = '\0'; 
    try {
        ordered_json requestData = ordered_json::parse(buf); 

        if(requestData.size() != 1) {
            return 1;
        }

        auto it = requestData.begin();
        pipe_id = it.key();
        pipeline_data = it.value();

        return 0;  
    } 
    catch(json::parse_error &e) {
        std::cerr << e.what() << std::endl;
        return 1;  
    }
}
