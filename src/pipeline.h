#ifndef __M2E_BRIDGE_PIPELINE_H__
#define __M2E_BRIDGE_PIPELINE_H__


#include <string>
#include <vector>
#include <thread>

#include "aliases.h"
#include "connectors/connector.h"
#include "filters/filter.h"
#include "transformers/transformer.h"
#include "m2e_message/message_wrapper.h"


class Pipeline {
public:
    Pipeline(std::string const & pipeid, json const & pjson);
    void start();
    void stop();
private:
    void run();
    bool filter(MessageWrapper& msg_w);
    void transform(MessageWrapper& msg_w);
    void map(MessageWrapper& msg_w);
    Connector* connector_in_;
    Connector* connector_out_;
    std::vector<Filter*> filters_;
    std::vector<Transformer*> transformers_;
    std::string pipeid_;

    bool stop_ { false };
    std::thread *th_ { nullptr };
};


#endif
