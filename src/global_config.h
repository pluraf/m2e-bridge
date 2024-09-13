#ifndef __M2E_BRIDGE_GLOBAL_CONFIG_H__
#define __M2E_BRIDGE_GLOBAL_CONFIG_H__


#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

#include "m2e_aliases.h"


class GlobalConfig {
public:
    std::string get_jwt_public_key_path(){
        return config_.at("jwt_public_key_path").get<std::string>();
    }

    json const & get_pipelines_config(){
        return pipelines_;
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

        pipelines_ = json::parse(buffer.str());
    }

private:
    json config_;
    json pipelines_;
};


extern GlobalConfig gc;


#endif  // __M2E_BRIDGE_GLOBAL_CONFIG_H__