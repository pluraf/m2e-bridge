#ifndef __M2E_BRIDGE_LIMITER_FT_H__
#define __M2E_BRIDGE_LIMITER_FT_H__


#include <cstdlib>
#include <ctime>

#include "filtra.h"


class LimiterFT:public Filtra{
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

private:
    unsigned long size_ {0};
};


#endif  // __M2E_BRIDGE_LIMITER_FT_H__