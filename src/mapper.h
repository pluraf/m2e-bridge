#ifndef __M2E_BRIDGE_MAPPER_H__
#define __M2E_BRIDGE_MAPPER_H__


#include <vector>
#include <utility>

#include <nlohmann/json.hpp>

#include "condition.h"
#include "router.h"
#include "connector.h"
#include "m2e_message/message_wrapper.h"


class Mapper {
public:
    Mapper(nlohmann::json json_desr) {
        using json = nlohmann::json;
    }
    void apply(MessageWrapper const &msg_w);
    void start() {}
    void stop() {}
private:
    Condition condition_;
    Router router_;
    Connector connector_out_;
};


#endif