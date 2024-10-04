#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <stdexcept>

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
    AuthHandler(std::string* public_key):public_key_(public_key) {}

    bool authorize(CivetServer *server, struct mg_connection *conn) override {
        return 1;
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
};


class PipelineApiHandler:public CivetHandler {
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
                const char * pipeid = last_segment + 1;
                try{
                    if(!ps->add_pipeline(pipeid, pipeline_data)){
                        mg_send_http_error(conn, 500, "Failed to add pipeline!");
                    }else{
                        mg_send_http_ok(conn, "text/plain", 0);
                    }
                }catch(std::invalid_argument const & e){
                    mg_send_http_error(conn, 422, "%s", e.what());
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
                mg_send_http_error(conn, 404, "%s", e.what());
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
        }else{
            mg_send_http_error(conn, 400, "Pipeline ID is missing in URI");
        }
        return true;
    }

    bool handleDelete(CivetServer * server, struct mg_connection * conn)override{
        std::vector<std::string> pipeline_ids;
        PipelineSupervisor *ps = PipelineSupervisor::get_instance();
        if(parse_pipeline_ids(conn, pipeline_ids) != 0){
            mg_send_http_error(conn, 400, "Could not parse request!");
        }else{
            // TODO: Write changes to the disk in the end of the loop
            json deleted;
            deleted["deleted"] = json::array();
            for(const auto &pipeid : pipeline_ids){
                try{
                    if(!ps->delete_pipeline(pipeid)) {
                        mg_send_http_error(conn, 500, "Failed to delete pipeline!");
                        return true;
                    }
                    deleted["deleted"].push_back(pipeid);
                }catch(std::invalid_argument){
                    continue;
                }
            }
            std::string serialized = deleted.dump();
            mg_send_http_ok(conn, "application/json", serialized.size());
            mg_write(conn, serialized.c_str(), serialized.size());
        }
        return true;
    }
};

class PipelineStateApiHandler:public CivetHandler {
public:
    bool handleGet(CivetServer * server, struct mg_connection * conn)override{
        std::string response;
        const struct mg_request_info * req_info = mg_get_request_info(conn);

        PipelineSupervisor *ps = PipelineSupervisor::get_instance();
        std::map<std::string, Pipeline> pipelines = ps->get_pipelines();

        const char * last_segment = strrchr(req_info->request_uri, '/');
        if(last_segment && strlen(last_segment) > 1){
            const char * pipeid = last_segment + 1;
            auto pos = pipelines.find(pipeid);
            if(pos != pipelines.end()){
               response = get_pipeline_state_as_json(pos->second).dump();      
            }else{
                mg_send_http_error(conn, 404, "%s", "Pipeid not found!");
            }
        }else{
            response = get_all_pipelines_state_as_json(pipelines).dump();
        }
        mg_send_http_ok(conn, "application/json", response.size());
        mg_write(conn, response.c_str(), response.size());
        return true;
    }

};


CivetServer* start_server() {
    static std::string public_key = load_public_key(gc.get_jwt_public_key_path());

    char const * options[] = {
        "listening_ports", "8002",
        "num_threads", "1",
        NULL
    };

    CivetServer* server = new CivetServer(options);

    server->addAuthHandler("/**", new AuthHandler(&public_key));
    server->addHandler("/pipeline/config/", new PipelineApiHandler());
    server->addHandler("/pipeline/state/", new PipelineStateApiHandler());

    return server;
}


void stop_server(CivetServer* server) {
    if (server) {
        delete server;
    }
}
