
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


void PipelineSupervisor::stop_all(){
    for(auto & el: pipelines_){
         el.second->stop();
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
