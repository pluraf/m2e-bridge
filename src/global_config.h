#ifndef __M2E_BRIDGE_GLOBAL_CONFIG_H__
#define __M2E_BRIDGE_GLOBAL_CONFIG_H__


#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <format>

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
            throw std::invalid_argument(format("Pipeline [ {} ] already exist!", pipeid));
        }
        pipelines_[pipeid] = pipelineData;
        return save_pipelines();
    }

    bool edit_pipeline(const std::string &pipeid, const json &pipelineData){
        if(! pipelines_.contains(pipeid)) {
            throw std::invalid_argument(format("Pipeline [ {} ] doesn't exist!", pipeid));
        }
        pipelines_[pipeid] = pipelineData;
        return save_pipelines();
    }

    bool delete_pipeline(const std::string &pipeid){
        if(! pipelines_.contains(pipeid)) {
            throw std::invalid_argument(format("Pipeline [ {} ] doesn't exist!", pipeid));
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