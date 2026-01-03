/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


#ifndef __M2E_BRIDGE_FILTRA_FACTORY__
#define __M2E_BRIDGE_FILTRA_FACTORY__


#include <iostream>

#include "m2e_aliases.h"
#include "filtras/all.h"


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
        }else if(json_descr["type"] == "splitter"){
            return new SplitterFT(pi, json_descr);
        }else if(json_descr["type"] == "limiter"){
            return new LimiterFT(pi, json_descr);
        }else if(json_descr["type"] == "nop"){
            return new NopFT(pi, json_descr);
        }else if(json_descr["type"] == "throttle"){
            return new ThrottleFT(pi, json_descr);
        }else if(json_descr["type"] == "extractor"){
            return new ExtractorFT(pi, json_descr);
        }else if(json_descr["type"] == "converter"){
            return new ConverterFT(pi, json_descr);
        }else if(json_descr["type"] == "image"){
            return new ImageFT(pi, json_descr);
        }else{
            throw std::invalid_argument("Unknown filtra");
        }
    }
};


#endif  // __M2E_BRIDGE_FILTRA_FACTORY__