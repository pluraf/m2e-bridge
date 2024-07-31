#ifndef __M2E_BRIDGE_TRANSFORMER_H__
#define __M2E_BRIDGE_TRANSFORMER_H__


#include <nlohmann/json.hpp>


class Transformer {
public:
    Transformer(nlohmann::json json_descr) {
        using json = nlohmann::json;
    }

    void apply(MessageWrapper &msg_w) {}
};


#endif