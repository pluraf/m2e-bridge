
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


#include "pipeline_supervisor.h"
#include "global_config.h"


PipelineSupervisor * PipelineSupervisor::instance_ = nullptr;


void PipelineSupervisor::init(){
    json const & config_pipelines = gc.get_pipelines_config();
    // Iterate over the array of pipelines
    for(auto it = config_pipelines.begin(); it != config_pipelines.end(); ++it){
        pipelines_.emplace(it.key(), new Pipeline(it.key(), * it));
    }
}


void PipelineSupervisor::start_all(){
    for(auto & el: pipelines_){
         el.second->start();
    }
}


void PipelineSupervisor::terminate_all(){
    for(auto & el: pipelines_){
         el.second->terminate();
    }
}


bool PipelineSupervisor::add_pipeline(string const & pipeid, json const & pipeline_data){
    auto pos = pipelines_.find(pipeid);
    if(pos != pipelines_.end()){
        return false;
    }
    if(gc.add_pipeline(pipeid, pipeline_data) != 0){
        return false;
    }
    pipelines_.emplace(pipeid, new Pipeline(pipeid, pipeline_data));
    return true;
}


bool PipelineSupervisor::delete_pipeline(string const & pipeid){
    get_pipeline(pipeid)->terminate();
    if(gc.delete_pipeline(pipeid) != 0){
        return false;
    }
    pipelines_.erase(pipeid);
    return true;
}


bool PipelineSupervisor::edit_pipeline(string const & pipeid, json const & pipeline_data){
    Pipeline * pipeline = get_pipeline(pipeid);
    PipelineState pstate = pipeline->get_state();
    bool res = delete_pipeline(pipeid) && add_pipeline(pipeid, pipeline_data);
    if(res){
        if(pstate != PipelineState::STOPPED && pstate != PipelineState::STOPPING){
            get_pipeline(pipeid)->start();
        }
    }
    return res;
}


Pipeline * PipelineSupervisor::get_pipeline(string const & pipeid){
    auto pos = pipelines_.find(pipeid);
    if(pos == pipelines_.end()){
        throw std::out_of_range(fmt::format("pipeid [ {} ]is not found", pipeid));
    }
    return pos->second;
}


map<string, Pipeline *> const & PipelineSupervisor::get_pipelines()const{
    return pipelines_;
}
