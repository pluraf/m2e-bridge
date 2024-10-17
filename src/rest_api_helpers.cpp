#include <iostream>
#include <fstream>

#include "rest_api_helpers.h"


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

PipelineCommand parse_pipeline_command(struct mg_connection *conn, json & state_data){
    std::string command_string = "";
    if(parse_request_body(conn, state_data) != 0){
        return PipelineCommand::NONE;
    }
    try{
        command_string = state_data.at("state");
        return pipeline_command_from_string(command_string);
    }catch(json::exception){
        return PipelineCommand::NONE;
    }

}

json get_pipeline_state_as_json(const Pipeline& pipeline){

    json json_object = json{
        {"state", pipeline_state_to_string(pipeline.get_state())}
    };
    if(pipeline.get_state() == PipelineState::FAILED || 
        pipeline.get_state() == PipelineState::MALFORMED ){
        json_object["error"] = pipeline.get_last_error();
    }
    return json_object;
}

json get_all_pipelines_state_as_json(const std::map<std::string, Pipeline> & pipelines){
    json json_object;
    for( auto const& [key, val]: pipelines){
        json_object[key] = get_pipeline_state_as_json(val);
    }
    return json_object;
}
