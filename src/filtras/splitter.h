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


#ifndef __M2E_BRIDGE_SPLITTER_FT_H__
#define __M2E_BRIDGE_SPLITTER_FT_H__


#include <cstdlib>
#include <ctime>

#include "filtra.h"


class SplitterFT:public Filtra{
public:
    SplitterFT(PipelineIface const & pi, json const & json_descr):
            Filtra(pi, json_descr){
        chunk_size_ = json_descr.at("chunk_size");
        // Generate first random message id
        std::srand(static_cast<unsigned>(std::time(0)));
        message_id_ = std::rand() % 65536;
    }

    string process_message(MessageWrapper & msg_w)override{
        string const & data = msg_w.msg().get_raw();
        if(data.size() > chunk_size_){
            msg_w_ = & msg_w;
            chunk_counter_ = 0;
            ++message_id_;
            return "self";
        }else{
            msg_w.pass();
            return "";
        }
    }

    Message generate_message()override{
        if(chunk_counter_ == -1) return Message();
        auto j_chunk = json::array();
        string const & orig_data = msg_w_->msg().get_raw();
        auto chunk_start = orig_data.begin() + chunk_size_ * chunk_counter_;
        auto rest = orig_data.end() - chunk_start;
        if(rest <= 0) return Message();
        rest = rest > chunk_size_ ? chunk_size_ : rest;
        vector<uint8_t> buffer(chunk_start, chunk_start + rest);
        j_chunk.push_back("____SPL");
        j_chunk.push_back(message_id_);
        j_chunk.push_back(orig_data.size());
        j_chunk.push_back(chunk_counter_++);
        j_chunk.push_back(json::binary(std::move(buffer)));
        return Message(j_chunk, MessageFormat::Type::CBOR, msg_w_->msg().get_topic());
    }

    static pair<string, json> get_schema(){
        json schema = Filtra::get_schema();
        schema.merge_patch({
            {"chunk_size", {
                {"type", "integer"},
                {"required", true}
            }}
        });
        return {"splitter", schema};
    }

private:
    unsigned long chunk_size_ {0};
    unsigned long message_id_ {0};
    long chunk_counter_ {-1};
    vector<uint8_t> buffer_;
    MessageWrapper * msg_w_ {nullptr};
};


#endif  // __M2E_BRIDGE_SPLITTER_FT_H__