#ifndef __M2E_BRIDGE_FILTER_H__
#define __M2E_BRIDGE_FILTER_H__


#include "nlohmann/json.hpp"


class Filter {
public:
    Filter(nlohmann::json json_descr) {
        using json = nlohmann::json;
    }

    bool apply(const MessageWrapper &msg_w) { return true; }
};


#endif