#include <iostream>
#include <cstring>
#include <fstream>

#include "rest_api.h"
#include <jwt-cpp/jwt.h>

#include "jwt_helpers.h"

class authHandler:public CivetAuthHandler {
    public:
        authHandler(std::string* public_key):public_key_(public_key) {}

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


CivetServer* start_server() {
    static std::string public_key = load_public_key("../public_key.pem");

    CivetServer* server = new CivetServer(NULL);

    authHandler* auth_handler = new authHandler(&public_key);

    server->addAuthHandler("/**", auth_handler);
    
    return server;
}

void stop_server(CivetServer* server) {
    if (server) {
        delete server;
    }
}
