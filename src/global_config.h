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
    string get_jwt_public_key_path(){
        return config_.at("jwt_public_key_path").get<std::string>();
    }

    string const & get_authbundles_db_path(){
        return authbundles_db_path_;
    }

    string const & get_converters_db_path(){
        return converters_db_path_;
    }

    ordered_json const & get_pipelines_config(){
        return pipelines_;
    }

    ordered_json const & get_http_gate_config(){
        return http_gate_;
    }

    json const & get_shared_objects_config(){
        return shared_objects_;
    }

    bool get_api_authentication(){
        return config_.value("api_authentication", true);
    }

    string const & get_ca_storage(){
        static string ca_storage {"/gnode/storage/ca/"};
        return ca_storage;
    }

    bool set_api_authentication(bool value){
        config_["api_authentication"] = value;
        return save_config();
    }

    bool add_pipeline(string const & pipeid, json const & pipelineData) {
        if(pipelines_.contains(pipeid)) {
            throw std::invalid_argument(fmt::format("Pipeline [ {} ] already exist!", pipeid));
        }
        pipelines_[pipeid] = pipelineData;
        return save_pipelines();
    }

    bool update_pipeline(string const & pipeid, json const & pipelineData){
        if(! pipelines_.contains(pipeid)) {
            throw std::invalid_argument(fmt::format("Pipeline [ {} ] doesn't exist!", pipeid));
        }
        pipelines_[pipeid].merge_patch(pipelineData);
        return save_pipelines();
    }

    bool delete_pipeline(string const& pipeid){
        if(! pipelines_.contains(pipeid)) {
            throw std::invalid_argument(fmt::format("Pipeline [ {} ] doesn't exist!", pipeid));
        }
        pipelines_.erase(pipeid);
        return save_pipelines();
    }

    bool save_config(){
        auto data = config_.dump(4);
        std::ofstream ofs(config_path_);
        if(! ofs.is_open()) return false;
        ofs << data;
        ofs.close();
        return true;
    }

    bool save_pipelines(){
        auto data = pipelines_.dump(4);
        std::string pipelines_path = config_.at("pipelines_path").get<std::string>();
        std::ofstream ofs(pipelines_path);
        if(! ofs.is_open()) return false;
        ofs << data;
        ofs.close();
        return true;
    }

    bool save_http_gate(json const & config){
        auto data = config.dump(4);
        string const & http_gate_path = config_.at("http_gate_path").get<std::string>();
        std::ofstream ofs(http_gate_path);
        if(! ofs.is_open()) return false;
        ofs << data;
        ofs.close();
        return true;
    }

    void load(std::string const & config_path){
        config_path_ = config_path;

        ///////////////////
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

        /////////////////
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

        //////////////////////
        // Load shared objects
        std::string shared_objects_path = config_.at("shared_objects_path").get<std::string>();
        file = std::ifstream(shared_objects_path);
        if(!file){
            std::cerr << "Failed to open file: " << shared_objects_path << std::endl;
            throw std::runtime_error("");
        }
        buffer = std::stringstream();
        buffer << file.rdbuf();
        file.close();
        shared_objects_ = json::parse(buffer.str());

        /////////////////
        // Load HTTP Gate
        std::string http_gate_path = config_.at("http_gate_path").get<std::string>();
        file = std::ifstream(http_gate_path);
        if(! file){
            std::cerr << "Failed to open file: " << http_gate_path << std::endl;
            throw std::runtime_error("");
        }
        buffer = std::stringstream();
        buffer << file.rdbuf();
        file.close();
        http_gate_ = ordered_json::parse(buffer.str());

        ///////////////////////////
        // Load authbundles_db_path
        authbundles_db_path_ = config_.at("authbundles_db_path").get<std::string>();

        //////////////////////////
        // Load converters_db_path
        converters_db_path_ = config_.at("converters_db_path").get<std::string>();
    }
private:
    json config_;
    ordered_json pipelines_;
    ordered_json http_gate_;
    json shared_objects_;
    std::string authbundles_db_path_;
    std::string converters_db_path_;
    std::string config_path_;
};


extern GlobalConfig gc;


#endif  // __M2E_BRIDGE_GLOBAL_CONFIG_H__