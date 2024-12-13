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


#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <stdexcept>
#include <future>

#include "jwt-cpp/jwt.h"
#include "global_config.h"
#include "rest_api.h"
#include "jwt_helpers.h"
#include "pipeline.h"
#include "pipeline_supervisor.h"
#include "rest_api_helpers.h"
#include "global_config.h"


class AuthHandler:public CivetAuthHandler{
public:
    AuthHandler(std::string* public_key){
        public_key_ = public_key;
    }

    bool authorize(CivetServer *server, struct mg_connection *conn) override {
        allow_anonymous_ = ! gc.get_api_authentication();
        if(allow_anonymous_) return 1;

        const char* auth_token = mg_get_header(conn, "Authorization");
        if (auth_token != NULL && strlen(auth_token) > 7) {
            const char *token = auth_token + 7;  // Skip "Bearer "

            if (jwt_verify(token, *public_key_)) {
                return 1;
            }
        }

        mg_send_http_error(conn, 401, "");
        return 0;
    }

private:
    std::string* public_key_;
    bool allow_anonymous_ {false};
};


class PipelineConfigApiHandler:public CivetHandler{
public:
    bool handlePost(CivetServer * server, struct mg_connection * conn)override{
        json pipeline_data;
        const struct mg_request_info * req_info = mg_get_request_info(conn);
        PipelineSupervisor *ps = PipelineSupervisor::get_instance();
        const char * last_segment = strrchr(req_info->request_uri, '/');
        if(last_segment && strlen(last_segment) > 1){
            if(parse_request_body(conn, pipeline_data) != 0){
                mg_send_http_error(conn, 400, "Could not parse request!");
            }else{
                if(! gc.validate_pipeline_data(pipeline_data)){
                    mg_send_http_error(conn, 422, "Invalid pipeline configuration!");
                }
                else{
                    const char * pipeid = last_segment + 1;
                    try{
                        if(!ps->add_pipeline(pipeid, pipeline_data)){
                            mg_send_http_error(conn, 500, "Failed to add pipeline!");
                        }
                        else{
                            mg_send_http_ok(conn, "text/plain", 0);
                        }
                    }catch(std::invalid_argument const & e){
                        mg_send_http_error(conn, 422, "%s", e.what());
                    }
                }
            }
        }else{
            mg_send_http_error(conn, 400, "Pipeline ID is missing in URI");
        }
        return true;
    }

    bool handleGet(CivetServer * server, struct mg_connection * conn)override{
        json pipeline_data;
        const struct mg_request_info * req_info = mg_get_request_info(conn);

        ordered_json pipelines = gc.get_pipelines_config();

        const char * last_segment = strrchr(req_info->request_uri, '/');
        if(last_segment && strlen(last_segment) > 1){
            const char * pipeid = last_segment + 1;
            try{
               std::string serialized = pipelines.at(pipeid).dump();
               mg_send_http_ok(conn, "application/json", serialized.size());
               mg_write(conn, serialized.c_str(), serialized.size());
            }catch(json::exception const & e){
                mg_send_http_error(conn, 404, "");
            }
        }else{
            std::string serialized = pipelines.dump();
            mg_send_http_ok(conn, "application/json", serialized.size());
            mg_write(conn, serialized.c_str(), serialized.size());
        }
        return true;
    }

    bool handlePut(CivetServer * server, struct mg_connection * conn)override{
        json pipeline_data;
        const struct mg_request_info * req_info = mg_get_request_info(conn);
        PipelineSupervisor *ps = PipelineSupervisor::get_instance();
        const char * last_segment = strrchr(req_info->request_uri, '/');
        if(last_segment && strlen(last_segment) > 1){
            if(parse_request_body(conn, pipeline_data) != 0){
                mg_send_http_error(conn, 400, "Could not parse request!");
            }else{
                if(!gc.validate_pipeline_data(pipeline_data)){
                    mg_send_http_error(conn, 422, "Invalid pipeline configuration!");
                }else{
                    const char * pipeid = last_segment + 1;
                    try{
                        if(!ps->edit_pipeline(pipeid, pipeline_data)){
                            mg_send_http_error(conn, 500, "Failed to edit pipeline!");
                        }else{
                            mg_send_http_ok(conn, "text/plain", 0);
                        }
                    }catch(std::invalid_argument const & e){
                        mg_send_http_error(conn, 404, "%s", e.what());
                    }
                }
            }
        }else{
            mg_send_http_error(conn, 400, "Pipeline ID is missing in URI");
        }
        return true;
    }

    bool handleDelete(CivetServer * server, struct mg_connection * conn)override{
        std::vector<std::string> pipeline_ids;
        const struct mg_request_info * req_info = mg_get_request_info(conn);
        PipelineSupervisor * ps = PipelineSupervisor::get_instance();

        const char * last_segment = strrchr(req_info->request_uri, '/');
        if(last_segment && strlen(last_segment) > 1){
            char const * pipeid = last_segment + 1;
            try{
                if(! ps->delete_pipeline(pipeid)){
                    mg_send_http_error(conn, 500, "Failed to delete pipeline!");
                    return true;
                }
            }catch(std::invalid_argument){
                mg_send_http_error(conn, 404, "");
                return true;
            }
            mg_send_http_ok(conn, "plain/text", 0);
        }else{
            mg_send_http_error(conn, 400, "Pipeline ID is missing in URI");
        }
        return true;
    }
};


class PipelineStatusApiHandler:public CivetHandler{
public:
    bool handleGet(CivetServer * server, struct mg_connection * conn)override{
        std::string response;
        const struct mg_request_info * req_info = mg_get_request_info(conn);

        PipelineSupervisor *ps = PipelineSupervisor::get_instance();
        auto const & pipelines = ps->get_pipelines();

        const char * last_segment = strrchr(req_info->request_uri, '/');
        if(last_segment && strlen(last_segment) > 1){
            const char * pipeid = last_segment + 1;
            auto pos = pipelines.find(pipeid);
            if(pos != pipelines.end()){
               response = get_pipeline_status(* pos->second).dump();
            }else{
                mg_send_http_error(conn, 404, "%s", "Pipeid not found!");
            }
        }else{
            response = create_response(pipelines).dump();
        }
        mg_send_http_ok(conn, "application/json", response.size());
        mg_write(conn, response.c_str(), response.size());
        return true;
    }

private:
    json get_pipeline_status(Pipeline const & pipeline){
        PipelineState state = pipeline.get_state();
        json j_status = json{
            {"status", Pipeline::state_to_string(state)}
        };
        string error = pipeline.get_last_error();
        if(! error.empty()){
            j_status["error"] = error;
        }
        PipelineStat stat = pipeline.get_statistics();
        j_status["last_in"] = stat.last_in;
        j_status["last_out"] = stat.last_out;
        j_status["count_in"] = stat.count_in;
        j_status["count_out"] = stat.count_out;
        return j_status;
    }
    json create_response(map<string, Pipeline *> const & pipelines){
        json json_object;
        for( auto const& [key, val]: pipelines){
            json_object[key] = get_pipeline_status(* val);
        }
        return json_object;
    }
};


class PipelineControlApiHandler:public CivetHandler{
    bool handlePut(CivetServer * server, struct mg_connection * conn)override{
        json state_data;
        const struct mg_request_info * req_info = mg_get_request_info(conn);

        vector<string> segments;
        try{
            segments = get_last_segments(req_info->request_uri, 2);
        }catch(std::out_of_range){
            mg_send_http_error(conn, 400, "");
        }

        string const & pipeid = segments[1];
        string const & command_str = segments[0];

        PipelineCommand command = Pipeline::command_from_string(command_str);

        if(command != PipelineCommand::START
                && command != PipelineCommand::STOP
                && command != PipelineCommand::RESTART){
            mg_send_http_error(conn, 400, "Command unknown");
        }else{
            PipelineSupervisor * ps = PipelineSupervisor::get_instance();
            try{
                ps->get_pipeline(pipeid)->execute(command);
                mg_send_http_ok(conn, "text/plain", 0);
            }catch(std::out_of_range){
                mg_send_http_error(conn, 404, "%s", "Pipeline not found!");
            }
        }
        return true;
    }
};


class PipelineComponentsApiHandler:public CivetHandler{
    bool handleGet(CivetServer * server, struct mg_connection * conn)override{
        std::string response;
        const struct mg_request_info * req_info = mg_get_request_info(conn);

        try{
            nlohmann::json pipeline_components = get_schemas();
            response = pipeline_components.dump(4);
        }catch(json::exception const & e){
                mg_send_http_error(conn, 404, "");
        }

        mg_send_http_ok(conn, "application/json", response.size());
        mg_write(conn, response.c_str(), response.size());
        return true;
    }
};

CivetServer* start_server(){
    static std::string public_key = load_public_key(gc.get_jwt_public_key_path());

    char const * options[] = {
        "listening_ports", "8002",
        "num_threads", "1",
        NULL
    };

    CivetServer* server = new CivetServer(options);

    server->addAuthHandler("/**", new AuthHandler(&public_key));
    server->addHandler("/pipeline/config/", new PipelineConfigApiHandler());
    server->addHandler("/pipeline/status/", new PipelineStatusApiHandler());
    server->addHandler("/pipeline/control/", new PipelineControlApiHandler());
    server->addHandler("/pipeline/pipeline_components/", new PipelineComponentsApiHandler());

    return server;
}


void stop_server(CivetServer* server){
    if (server) {
        delete server;
    }
}
