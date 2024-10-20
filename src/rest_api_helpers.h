#ifndef __M2E_BRIDGE_REST_API_HELPERS_H__
#define __M2E_BRIDGE_REST_API_HELPERS_H__


#include <string>
#include "CivetServer.h"
#include "pipeline.h"

#include "m2e_aliases.h"


std::vector<std::string> get_last_segments(char const * uri, int count);
int parse_request_body(mg_connection * conn, json & pipelineData);
int parse_pipeline_ids(mg_connection * conn, std::vector<std::string> & pipeline_ids);
json get_pipeline_state_as_json(const Pipeline& pipeline);
json get_all_pipelines_state_as_json(const std::map<std::string, Pipeline *> & pipelines);


#endif  // __M2E_BRIDGE_REST_API_HELPERS_H__
