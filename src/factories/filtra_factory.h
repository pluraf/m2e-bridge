#ifndef __M2E_BRIDGE_FILTRA_FACTORY__
#define __M2E_BRIDGE_FILTRA_FACTORY__


#include <iostream>

#include "m2e_aliases.h"
#include "filtras/comparator.h"
#include "filtras/finder.h"
#include "filtras/eraser.h"
#include "filtras/builder.h"


class FiltraFactory {
public:
    static Filtra * create(PipelineIface const & pi, json const & json_descr) {
        if(json_descr["type"] == "comparator"){
            return new ComparatorFT(pi, json_descr);
        }else if(json_descr["type"] == "finder"){
            return new FinderFT(pi, json_descr);
        }else if(json_descr["type"] == "eraser"){
            return new EraserFT(pi, json_descr);
        }else if(json_descr["type"] == "builder"){
            return new BuilderFT(pi, json_descr);
        }else{
            throw std::invalid_argument("Unknown filtra");
        }
    }
};


#endif  // __M2E_BRIDGE_FILTRA_FACTORY__