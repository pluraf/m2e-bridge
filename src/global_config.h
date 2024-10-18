#ifndef __M2E_BRIDGE_GLOBAL_CONFIG_H__
#define __M2E_BRIDGE_GLOBAL_CONFIG_H__


#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fmt/core.h>

#include "m2e_aliases.h"



class GlobalConfig {
public:
    std::string get_jwt_public_key_path(){
        return config_.at("jwt_public_key_path").get<std::string>();
    }

    std::string const & get_authbundles_db_path(){
        return authbundles_db_path_;
    }

    ordered_json const & get_pipelines_config(){
        return pipelines_;
    }

    bool add_pipeline(const std::string &pipeid, const json &pipelineData) {
        if(pipelines_.contains(pipeid)) {
            throw std::invalid_argument(fmt::format("Pipeline [ {} ] already exist!", pipeid));
        }
        pipelines_[pipeid] = pipelineData;
        return save_pipelines();
    }

    bool update_pipeline(const std::string &pipeid, const json &pipelineData){
        if(! pipelines_.contains(pipeid)) {
            throw std::invalid_argument(fmt::format("Pipeline [ {} ] doesn't exist!", pipeid));
        }
        pipelines_[pipeid] = pipelineData;
        return save_pipelines();
    }

    bool delete_pipeline(const std::string &pipeid){
        if(! pipelines_.contains(pipeid)) {
            throw std::invalid_argument(fmt::format("Pipeline [ {} ] doesn't exist!", pipeid));
        }
        pipelines_.erase(pipeid);
        return save_pipelines();
    }

    bool save_pipelines() {
        std::string pipelines_path = config_.at("pipelines_path").get<std::string>();
        std::ofstream ofs(pipelines_path);
        if(! ofs.is_open()) {
            return 1;
        }
        ofs << pipelines_.dump(4);
        ofs.close();
        return 0;
    }

    bool validate_connector(const json& connector) {
        if(!connector.contains("type") || !connector["type"].is_string()) {
            return false;
        }

        std::string type = connector["type"];
        const char* allowed_params_mqtt[] = {"type", "topic", "server", "qos", "retry_attempts", "authbundle_id", "client_id"};
        const char* allowed_params_gcp_pubsub[] = {"type", "authbundle_id", "project_id", "topic_id", "subscription_id", "attributes"};

        const char** allowed_params = nullptr;
        size_t allowed_params_size = 0;

        // Essential parameters
        if(type == "mqtt") {
            allowed_params = allowed_params_mqtt;
            allowed_params_size = sizeof(allowed_params_mqtt) / sizeof(allowed_params_mqtt[0]);

            if(!connector.contains("topic") || !connector["topic"].is_string() ||
               !connector.contains("server") || !connector["server"].is_string()) {
                return false;
               }
        }else if(type == "gcp_pubsub") {
            allowed_params = allowed_params_gcp_pubsub;
            allowed_params_size = sizeof(allowed_params_gcp_pubsub) / sizeof(allowed_params_gcp_pubsub[0]);

            if (!connector.contains("authbundle_id") || !connector["authbundle_id"].is_string() ||
                !connector.contains("project_id") || !connector["project_id"].is_string()) {
                    return false;
                }
        }else {
            // Invalid connector type
            return false;
        }

        // Unexpected parameters
        for(const auto& param : connector.items()) {
            bool is_allowed = false;
            for(size_t i = 0; i < allowed_params_size; i++) {
                if(param.key() == allowed_params[i]) {
                    is_allowed = true;
                    break;
                }
            }
            if(!is_allowed) {
                return false;
            }
        }

        return true;
    }

    bool validate_pipeline_data(const json &pipelineData) {
        if(!pipelineData.contains("connector_in") || !pipelineData["connector_in"].is_object() ||
           !pipelineData.contains("filters") || !pipelineData.contains("transformers") ||
           !pipelineData.contains("connector_out") || !pipelineData["connector_out"].is_object()) {
            return false;
        }

        // Validate connectors
        if(!validate_connector(pipelineData["connector_in"]) ||
           !validate_connector(pipelineData["connector_out"])) {
            return false;
        }

        return true;
    }

    void load(std::string const & config_path){
        // Load main config
        std::ifstream file(config_path);
        if(! file){
            std::cerr << "Failed to open file: " << config_path << std::endl;
            throw std::runtime_error("");
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        config_ = json::parse(buffer.str());

        // Load pipelines
        std::string pipelines_path = config_.at("pipelines_path").get<std::string>();
        file = std::ifstream(pipelines_path);
        if(! file){
            std::cerr << "Failed to open file: " << pipelines_path << std::endl;
            throw std::runtime_error("");
        }
        buffer = std::stringstream();
        buffer << file.rdbuf();
        file.close();

        pipelines_ = ordered_json::parse(buffer.str());

        // Load authbundles_db_path
        authbundles_db_path_ = config_.at("authbundles_db_path").get<std::string>();

    }

private:
    json config_;
    ordered_json pipelines_;
    std::string authbundles_db_path_;
};


extern GlobalConfig gc;


#endif  // __M2E_BRIDGE_GLOBAL_CONFIG_H__