#ifndef __M2E_BRIDGE_PIPELINE_IFACE_H__
#define __M2E_BRIDGE_PIPELINE_IFACE_H__


#include <chrono>


class PipelineIface{
public:
    virtual void schedule_event(std::chrono::milliseconds msec) = 0;
};

#endif  // __M2E_BRIDGE_PIPELINE_IFACE_H__