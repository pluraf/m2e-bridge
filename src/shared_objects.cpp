/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2025 Pluraf Embedded AB <code@pluraf.com>

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


#include <mutex>

#include "shared_objects.h"


void SharedObjects::init(json const & config)
{
    // Iterate over shared objects
    for(auto it = config.cbegin(); it != config.cend(); ++it){
        try{
            json const & obj = * it;
            string type = obj.contains("type") ? obj["type"] : "object";
            if(type == "object"){
                json_shared_objects_[it.key()] = obj["value"];
            }else if(type == "memory"){
                size_t size = obj["size"];
                byte * mem = new byte[size];
                memory_shared_objects_.emplace(it.key(), mem);
            }
        }catch(std::exception const & e){
            continue;
        }
    }
}


json & SharedObjects::get_json(string const & name)
{
    return get_instance().json_shared_objects_.at(name);
}


json const & SharedObjects::get_json()
{
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wreturn-local-addr"
    // get_instance() returns singleton, so it's safe to return it's member
    return get_instance().json_shared_objects_;
    #pragma clang diagnostic pop
}


byte * SharedObjects::get_memory(string const & name)
{
    return get_instance().memory_shared_objects_.at(name);
}