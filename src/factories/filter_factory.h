
#include "nlohmann/json.hpp"

#include "filters/filter.h"


class FilterFactory {
public:
    static Filter* create(nlohmann::json json_descr) {}
};