#ifndef __M2E_BRIDGE_FILTER_FACTORY__
#define __M2E_BRIDGE_FILTER_FACTORY__


#include <iostream>

#include "filters/comparator_filter.h"
#include "filters/search_filter.h"


class FilterFactory {
public:
    static Filter * create(json const & json_descr) {
        if(json_descr["operation"] == "compare"){
            return new ComparatorFilter(json_descr);
        }else if(json_descr["operation"] == "search"){
            return new SearchFilter(json_descr);
        }else{
            return new Filter(json_descr);
        }
    }
};


#endif  // __M2E_BRIDGE_FILTER_FACTORY__