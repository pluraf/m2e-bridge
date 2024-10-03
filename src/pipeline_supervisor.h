#ifndef __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__
#define __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__


#include <string>
#include <map>

#include "pipeline.h"


class PipelineSupervisor {

private:
    std::map<std::string, Pipeline> pipelines_;  // pipeid is a key
    static PipelineSupervisor* instance_;
    PipelineSupervisor() {} // private constructor to make class singleton
public:
    void init();
    void start();
    void stop();
    bool add_pipeline(std::string pipeid,json pipeline_data);
    bool delete_pipeline(std::string pipeid);
    bool edit_pipeline(std::string pipeid, json pipeline_data);
    std::map<std::string, Pipeline> get_pipelines();
    
    static PipelineSupervisor* get_instance(){
        if (instance_ == nullptr){
            instance_ = new PipelineSupervisor();
            instance_->init();
        }
        return instance_;
    }

};



#endif  // __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__