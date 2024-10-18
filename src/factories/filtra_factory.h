#ifndef __M2E_BRIDGE_FILTRA_FACTORY__
#define __M2E_BRIDGE_FILTRA_FACTORY__


#include <iostream>

#include "m2e_aliases.h"
#include "filters/comparator_filter.h"
#include "filters/finder_filter.h"
#include "transformers/eraser_transformer.h"


class FiltraFactory {
public:
    static Filtra * create(PipelineIface const & pi, json const & json_descr) {
        if(json_descr["type"] == "comparator"){
            return new ComparatorFilter(pi, json_descr);
        }else if(json_descr["type"] == "finder"){
            return new SearchFilter(pi, json_descr);
        }else if(json_descr["type"] == "eraser"){
            return new EraserTransformer(pi, json_descr);
        }else{
            throw std::invalid_argument("Unknown filtra");
        }
    }
};


#endif  // __M2E_BRIDGE_FILTRA_FACTORY__