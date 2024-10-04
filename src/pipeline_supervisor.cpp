#include "pipeline_supervisor.h"
#include "m2e_aliases.h"
#include "global_config.h"


void PipelineSupervisor::init(){
    json const & config_pipelines = gc.get_pipelines_config();
    // Iterate over the array of pipelines
    for(auto it = config_pipelines.begin(); it != config_pipelines.end(); ++it){
        pipelines_.emplace(it.key(), Pipeline(it.key(), *it));
    }
}


void PipelineSupervisor::start(){
    for(auto & el: pipelines_){
        el.second.start();
    }
}


void PipelineSupervisor::stop(){
    for(auto & el: pipelines_){
        el.second.stop();
    }
}

bool PipelineSupervisor::add_pipeline(std::string pipeid, json pipeline_data){
    auto pos = pipelines_.find(pipeid);
    if(pos != pipelines_.end()){
        return false;
    }
    if(gc.add_pipeline_in_config_file(pipeid, pipeline_data) != 0){
        return false;
    }
    pipelines_.emplace(pipeid, Pipeline(pipeid, pipeline_data));
    pipelines_.at(pipeid).start();
    return true;
}

bool PipelineSupervisor::delete_pipeline(std::string pipeid){
    auto pos = pipelines_.find(pipeid);
    if(pos == pipelines_.end()){
        return false;
    }
    if(gc.delete_pipeline_in_config_file(pipeid) != 0){
        return false;
    }
    pipelines_.at(pipeid).stop();
    pipelines_.erase(pipeid);
    return true;
}

bool PipelineSupervisor::edit_pipeline(std::string pipeid, json pipeline_data){
    auto pos = pipelines_.find(pipeid);
    if(pos == pipelines_.end()){
        return false;
    }
    if(gc.edit_pipeline_in_config_file(pipeid, pipeline_data) != 0){
        return false;
    }
    delete_pipeline(pipeid);
    add_pipeline(pipeid, pipeline_data);
    return true;
}

std::map<std::string, Pipeline> PipelineSupervisor::get_pipelines(){
    return pipelines_;
}
//Initialize static member to null
PipelineSupervisor* PipelineSupervisor::instance_ = nullptr;