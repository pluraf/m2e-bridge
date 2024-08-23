
#include "nlohmann/json.hpp"
#include <iostream>

#include "filters/filter.h"


class FilterFactory {
public:
    static Filter* create(nlohmann::json json_descr) {
        return new Filter(json_descr);
    }
};