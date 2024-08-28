
#ifndef __M2E_BRIDGE_TRFMR_FACTORY__
#define __M2E_BRIDGE_TRFMR_FACTORY__


#include "nlohmann/json.hpp"
#include <iostream>

#include "transformers/transformer.h"


class TransformerFactory {
public:
    static Transformer* create(nlohmann::json json_descr) {
        return new Transformer(json_descr);
    }
};

#endif