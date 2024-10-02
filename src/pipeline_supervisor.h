#ifndef __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__
#define __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__


#include <string>
#include <map>

#include "pipeline.h"


class PipelineSupervisor {
public:
    void init();
    void start();
    void stop();
    std::map<std::string, Pipeline> get_pipelines();
private:
    std::map<std::string, Pipeline> pipelines_;  // pipeid is a key
};


#endif  // __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__