/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024-2026 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_BRIDGE_LIMITER_FT_H__
#define __M2E_BRIDGE_LIMITER_FT_H__


#include <cstdlib>
#include <ctime>

#include "filtra.h"


//_DOCS: SECTION_START limiter_filtra Limiter Filtra
/*!
Rejects messages that exceed the specified size::

      {
          "type": "limiter",
          "size": <bytes>
      }
*/
//_DOCS: END

class LimiterFT: public Filtra
{
public:
    LimiterFT(PipelineIface const & pi, json const & json_descr):
            Filtra(pi, json_descr){
        size_ = json_descr.at("size");
    }

    string process_message(MessageWrapper & msg_w)override{
        string const & data = msg_w.msg().get_raw();
        msg_w.pass_if(data.size() <= size_);
        return "";
    }

    static pair<string, json> get_schema(){
        //_DOCS: SCHEMA_START limiter_filtra
        //_DOCS: SCHEMA_INCLUDE filtra
        static json schema = Filtra::get_schema({
            {"size", {
                {"type", "integer"},
                {"required", true},
                {"description", "Size limit in bytes for the message. Messages exceeding this limit are rejected."}
            }}
        });
        //_DOCS: END
        return {"limiter", schema};
    }

private:
    unsigned long size_ {0};
};


#endif  // __M2E_BRIDGE_LIMITER_FT_H__