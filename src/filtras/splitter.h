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

    string process(MessageWrapper & msg_w)override{
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

    Message process()override{
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
        return Message(j_chunk, msg_w_->msg().get_topic(), MessageFormat::CBOR);
    }

private:
    unsigned long chunk_size_ {0};
    unsigned long message_id_ {0};
    long chunk_counter_ {-1};
    vector<uint8_t> buffer_;
    MessageWrapper * msg_w_ {nullptr};
};


#endif  // __M2E_BRIDGE_SPLITTER_FT_H__