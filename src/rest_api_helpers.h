#ifndef __M2E_BRIDGE_REST_API_HELPERS_H__
#define __M2E_BRIDGE_REST_API_HELPERS_H__

#include <string>
#include <nlohmann/json.hpp>
#include "CivetServer.h"
#include "pipeline.h"

#include "m2e_aliases.h"

int parse_request_body(struct mg_connection * conn, json & pipelineData);
int parse_pipeline_ids(struct mg_connection * conn, std::vector<std::string> & pipeline_ids);
json get_pipeline_state_as_json(const Pipeline& pipeline);
json get_all_pipelines_state_as_json(const std::map<std::string, Pipeline *> & pipelines);
PipelineCommand parse_pipeline_command(struct mg_connection *conn, json & state_data);
#endif
