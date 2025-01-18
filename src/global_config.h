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

    bool get_api_authentication(){
        return config_.value("api_authentication", true);
    }

    bool set_api_authentication(bool value){
        config_["api_authentication"] = value;
        return save_config();
    }

    bool save_config(){
        std::ofstream ofs(config_path_);
        if(! ofs.is_open()) {
            return false;
        }
        ofs << config_.dump(4);
        ofs.close();
        return true;
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

    void load(std::string const & config_path){
        config_path_ = config_path;
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
    std::string config_path_;
};


extern GlobalConfig gc;


#endif  // __M2E_BRIDGE_GLOBAL_CONFIG_H__