#include "pipeline_supervisor.h"
#include "m2e_aliases.h"
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


void PipelineSupervisor::stop_all(){
    for(auto & el: pipelines_){
         el.second->stop();
    }
}


bool PipelineSupervisor::add_pipeline(std::string pipeid, json pipeline_data){
    auto pos = pipelines_.find(pipeid);
    if(pos != pipelines_.end()){
        return false;
    }
    if(gc.add_pipeline(pipeid, pipeline_data) != 0){
        return false;
    }
    pipelines_.emplace(pipeid, new Pipeline(pipeid, pipeline_data));
    pipelines_.at(pipeid)->start();
    return true;
}


bool PipelineSupervisor::delete_pipeline(std::string pipeid){
    auto pos = pipelines_.find(pipeid);
    if(pos == pipelines_.end()){
        return false;
    }
    if(gc.delete_pipeline(pipeid) != 0){
        return false;
    }
    pipelines_.erase(pipeid);
    return true;
}


bool PipelineSupervisor::edit_pipeline(std::string pipeid, json pipeline_data){
    return delete_pipeline(pipeid) && add_pipeline(pipeid, pipeline_data);
}


Pipeline & PipelineSupervisor::get_pipeline(std::string pipeid){
    auto pos = pipelines_.find(pipeid);
    if(pos == pipelines_.end()){
        throw std::out_of_range(fmt::format("pipeid [ {} ]is not found", pipeid));
    }
    return * pos->second;
}


map<string, Pipeline *> const & PipelineSupervisor::get_pipelines()const{
    return pipelines_;
}
