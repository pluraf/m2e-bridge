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


json get_pipeline_state_as_json(Pipeline const & pipeline){

    json json_object = json{
        {"status", pipeline_state_to_string(pipeline.get_state())}
    };
    if(pipeline.get_state() == PipelineState::FAILED ||
        pipeline.get_state() == PipelineState::MALFORMED ){
        json_object["error"] = pipeline.get_last_error();
    }
    return json_object;
}

json get_all_pipelines_state_as_json(map<string, Pipeline *> const & pipelines){
    json json_object;
    for( auto const& [key, val]: pipelines){
        json_object[key] = get_pipeline_state_as_json(* val);
    }
    return json_object;
}
