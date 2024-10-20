#ifndef __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__
#define __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__


#include "pipeline.h"


class PipelineSupervisor {
    PipelineSupervisor(){} // private constructor to make class singleton
    void init();

    map<string, Pipeline *> pipelines_;  // pipeid is a key
    static PipelineSupervisor * instance_;

public:

    void start_all();
    void stop_all();
    bool add_pipeline(string const & pipeid, json const & pipeline_data);
    bool delete_pipeline(string const & pipeid);
    bool edit_pipeline(string const & pipeid, json const & pipeline_data);
    map<string, Pipeline *> const & get_pipelines()const;
    Pipeline * get_pipeline(string const & pipeid);

public:
    static PipelineSupervisor * get_instance(){
        if (instance_ == nullptr){
            instance_ = new PipelineSupervisor();
            instance_->init();
        }
        return instance_;
    }
};


#endif  // __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__