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


#ifndef __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__
#define __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__


#include "pipeline.h"


class PipelineSupervisor {
    PipelineSupervisor(){} // private constructor to make class singleton
    void init();
    map<string, Pipeline *> pipelines_;  // pipeid is a key
    static PipelineSupervisor * instance_;
public:
    void start_all();
    void terminate_all();
    bool add_pipeline(string const & pipeid, json const & pipeline_data);
    bool delete_pipeline(string const & pipeid);
    bool edit_pipeline(string const & pipeid, json const & pipeline_data);
    map<string, Pipeline *> const & get_pipelines()const;
    Pipeline * get_pipeline(string const & pipeid);
    static PipelineSupervisor * get_instance(){
        if (instance_ == nullptr){
            instance_ = new PipelineSupervisor();
            instance_->init();
        }
        return instance_;
    }
};


#endif  // __M2E_BRIDGE_PIPELINE_SUPERVISOR_H__