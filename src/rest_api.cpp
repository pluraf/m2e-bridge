#include <iostream>
#include <cstring>
#include <fstream>

#include "jwt-cpp/jwt.h"

#include "global_config.h"
#include "rest_api.h"
#include "jwt_helpers.h"

#include "pipeline.h"
#include "rest_api_helpers.h"

#include "global_config.h"


class AuthHandler:public CivetAuthHandler {
    public:
        AuthHandler(std::string* public_key):public_key_(public_key) {}

        bool authorize(CivetServer *server, struct mg_connection *conn) override {
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

class CreatePipelineHandler:public CivetHandler {
    public:
        bool handlePost(CivetServer *server, struct mg_connection * conn) override {
            std::string pipeid;
            json pipeline_data;

            if(parse_request_body(conn, pipeid, pipeline_data) != 0) {
                mg_send_http_error(conn, 400, "Could not parse request!");
                return 0;
            }
        
            if(gc.add_pipeline(pipeid, pipeline_data) != 0) {
                mg_send_http_error(conn, 500, "Failed to add pipeline!");
                return 0;
            }

            return 1;
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

    AuthHandler* auth_handler = new AuthHandler(&public_key);
    server->addAuthHandler("/**", auth_handler);

    CreatePipelineHandler* create_pipeline_handler = new CreatePipelineHandler();
    server->addHandler("/pipeline/create", create_pipeline_handler);

    return server;
}

void stop_server(CivetServer* server) {
    if (server) {
        delete server;
    }
}
