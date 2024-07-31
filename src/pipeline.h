#ifndef __M2E_BRIDGE_PIPELINE_H__
#define __M2E_BRIDGE_PIPELINE_H__


#include <string>
#include <vector>
#include <thread>

#include "connector.h"
#include "filter.h"
#include "transformer.h"
#include "mapper.h"
#include "m2e_message/message_wrapper.h"


class Pipeline {
public:
    Pipeline(const std::string &json_str);
    void start();
    void stop();
private:
    void run();
    bool filter(MessageWrapper& msg_w);
    void transform(MessageWrapper& msg_w);
    void map(MessageWrapper& msg_w);
    Connector connector_in_;
    std::vector<Filter*> filters_;
    std::vector<Transformer*> transformers_;
    std::vector<Mapper*> mappers_;

    bool stop_ { false };
    std::thread *th_ { nullptr };
};


#endif
