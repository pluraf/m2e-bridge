#ifndef __M2E_BRIDGE_REST_API_HELPERS_H__
#define __M2E_BRIDGE_REST_API_HELPERS_H__

#include <string>
#include <nlohmann/json.hpp>
#include "CivetServer.h"

#include "m2e_aliases.h"


int parse_request_body(struct mg_connection * conn, std::string & pipeid, json & pipelineData);
int parse_pipeline_ids(struct mg_connection * conn, std::vector<std::string> & pipeline_ids);


#endif
