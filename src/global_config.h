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
#include <filesystem>
#include <fmt/core.h>

#include "m2e_aliases.h"


namespace fs = std::filesystem;


class PipelineConfigsIterator
{
    map<fs::path, ordered_json> const * configs_ { nullptr };
    map<fs::path, ordered_json>::const_iterator configs_iterator_;
    nlohmann::ordered_json::const_iterator pipelines_iterator_;

    public:
        using value_type = ordered_json;
        using iterator_category = std::forward_iterator_tag;
        using iterator_type = nlohmann::ordered_json::const_iterator;

        PipelineConfigsIterator() = default;

        PipelineConfigsIterator(map<fs::path, ordered_json> const * configs, bool end = false):
                configs_(configs)
        {
            if(end){
                configs_iterator_ = configs_->cend();
                if(configs_->size() > 0){
                    pipelines_iterator_ = (--(configs_->cend()))->second.cend();
                }
            }else{
                configs_iterator_ = configs_->cbegin();
                pipelines_iterator_ = configs_iterator_->second.cbegin();
            }
        }

        ordered_json const & operator*()
        {
            return *pipelines_iterator_;
        }

        PipelineConfigsIterator & operator++()
        {
            ++pipelines_iterator_;
            if(pipelines_iterator_ == configs_iterator_->second.cend()){
                ++configs_iterator_;
                if(configs_iterator_ != configs_->cend()){
                    pipelines_iterator_ = configs_iterator_->second.cbegin();
                }
            }
            return *this;
        }

        bool operator!=(PipelineConfigsIterator const & other)
        {
            return (configs_ != other.configs_
                    || configs_iterator_ != other.configs_iterator_
                    || pipelines_iterator_ != other.pipelines_iterator_);
        }

        string const & key()
        {
            return pipelines_iterator_.key();
        }
};


class PipelineConfigs{
    map<fs::path, ordered_json> configs_;
    fs::path default_path_;
public:
    void load_config(fs::path const & p, bool default_path = false)
    {
        std::ifstream file(p);
        if(! file){
            std::cerr << "Failed to open file: " << p << std::endl;
            throw std::runtime_error("");
        }
        std::stringstream buffer = std::stringstream();
        buffer << file.rdbuf();
        file.close();
        configs_[p] = ordered_json::parse(buffer.str());

        if(default_path){ default_path_ = p; }
    }

    bool save()
    {
        for(auto & config : configs_){
            auto data = config.second.dump(4);
            std::ofstream ofs(config.first);
            if(! ofs.is_open()) { return false; }
            ofs << data;
            ofs.close();
        }

        return true;
    }

    void erase(string const& pipeid)
    {
        for(auto & config : configs_){
            if(config.second.contains(pipeid)){
                config.second.erase(pipeid);
                return;
            }
        }
    }

    string dump()
    {
        json merged;
        for(auto & config : configs_){
            merged.merge_patch(config.second);
        }

        return merged.dump(4);
    }

    PipelineConfigsIterator cbegin() const
    {
        return PipelineConfigsIterator(&configs_);
    }

    PipelineConfigsIterator cend() const
    {
        return PipelineConfigsIterator(&configs_, true);
    }

    bool contains(string const & pipeid)
    {
        for(auto & config : configs_){
            if(config.second.contains(pipeid)) { return true; }
        }

        return false;
    }

    ordered_json & operator[](string const & pipeid)
    {
        for(auto & config : configs_){
            if(config.second.contains(pipeid)) { return config.second[pipeid]; }
        }

        return configs_[default_path_][pipeid];
    }

    ordered_json & at(string const & pipeid)
    {
        for(auto & config : configs_){
            if(config.second.contains(pipeid)) { return config.second[pipeid]; }
        }

        throw std::out_of_range(pipeid);
    }
};


class GlobalConfig{
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

    PipelineConfigs & get_pipelines_config(){
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
        return pipelines_.save();
    }

    bool update_pipeline(string const & pipeid, json const & pipelineData, bool override = false){
        if(! pipelines_.contains(pipeid)){
            throw std::invalid_argument(fmt::format("Pipeline [ {} ] doesn't exist!", pipeid));
        }

        if(override){
            pipelines_[pipeid] = pipelineData;
        }else{
            pipelines_[pipeid].merge_patch(pipelineData);
        }
        return pipelines_.save();
    }

    bool delete_pipeline(string const& pipeid){
        if(! pipelines_.contains(pipeid)) {
            throw std::invalid_argument(fmt::format("Pipeline [ {} ] doesn't exist!", pipeid));
        }
        pipelines_.erase(pipeid);
        return pipelines_.save();
    }

    bool save_config(){
        auto data = config_.dump(4);
        std::ofstream ofs(config_path_);
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
        pipelines_.load_config(pipelines_path, true);

        // If pipelines directory is specified, load all pipelines from it as well
        if(config_.contains("pipelines_dir_path")){
            std::string pipelines_dir_path = config_.at("pipelines_dir_path").get<std::string>();
            try{
                for(auto const & file_entry : fs::directory_iterator(pipelines_dir_path)){
                    if(file_entry.is_regular_file()){
                        pipelines_.load_config(file_entry.path());
                    }
                }
            }
            catch(const fs::filesystem_error& e){
                std::cerr << "Error accessing directory: " << e.what() << std::endl;
                throw std::runtime_error("");
            }
        }

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
    PipelineConfigs pipelines_;
    ordered_json http_gate_;
    json shared_objects_;
    std::string authbundles_db_path_;
    std::string converters_db_path_;
    std::string config_path_;
};


extern GlobalConfig gc;


#endif  // __M2E_BRIDGE_GLOBAL_CONFIG_H__