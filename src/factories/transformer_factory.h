
#include "nlohmann/json.hpp"
#include <iostream>

#include "transformers/transformer.h"


class TransformerFactory {
public:
    static Transformer* create(nlohmann::json json_descr) {
        return new Transformer(json_descr);
    }
};