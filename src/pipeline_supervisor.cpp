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

std::map<std::string, Pipeline> PipelineSupervisor::get_pipelines(){
    return pipelines_;
}
//Initialize static member to null
PipelineSupervisor* PipelineSupervisor::instance_ = nullptr;