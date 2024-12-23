#ifndef TEST_FILTRA_HELPER_H
#define TEST_FILTRA_HELPER_H

#include "../src/filtras/filtra.h"

class MockPipeline:public PipelineIface {
public:
    void schedule_event(std::chrono::milliseconds msec)override{
        std::cout << "MockPipeline: schedule_event called with " << msec.count() << "ms" << std::endl;
    }
};

#endif