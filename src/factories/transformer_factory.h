
#include "nlohmann/json.hpp"

#include "transformers/transformer.h"


class TransformerFactory {
public:
    static Transformer* create(nlohmann::json json_descr) {}
};