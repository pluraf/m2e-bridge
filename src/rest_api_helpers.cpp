#include <iostream>
#include <fstream>


#include "rest_api_helpers.h"


std::vector<std::string> get_last_segments(char const * uri, int count){
    vector<string> segments;
    string segment;
    std::stringstream ss(uri);
    while(std::getline(ss, segment, '/')){
        segments.push_back(segment);
    }
    if(segments.size() < count){
        throw std::out_of_range("");
    }
    return vector<string>(segments.end() - count, segments.end());
}

int parse_request_body(struct mg_connection * conn, json & pipeline_data){
    char buf[1024];
    int length = mg_read(conn, buf, sizeof(buf));
    buf[length] = '\0';
    try {
        pipeline_data = ordered_json::parse(buf);
        return 0;
    }
    catch(json::parse_error &e){
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

int parse_pipeline_ids(struct mg_connection *conn, std::vector<std::string> &pipeline_ids){
    char buf[1024];
    int length = mg_read(conn, buf, sizeof(buf));
    buf[length] = '\0';
    try {
        json requestData = json::parse(buf);

        if(! requestData.is_array()) return 1;

        for(const auto &item : requestData) {
            pipeline_ids.push_back(item.get<std::string>());
        }

        return 0;
    }
    catch (json::parse_error &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
