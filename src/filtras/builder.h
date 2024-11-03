#ifndef __M2E_BRIDGE_BUILDER_FT_H__
#define __M2E_BRIDGE_BUILDER_FT_H__


#include "filtra.h"


class BuilderFT:public Filtra{
public:
    BuilderFT(PipelineIface const & pi, json const & json_descr):
            Filtra(pi, json_descr){
        payload_ = json_descr.at("payload");
    }

    string process_message(MessageWrapper &msg_w)override{
        if(encoder_ == MessageFormat::JSON){
            msg_w.msg().get_json() = payload_;
        }else{
            throw std::runtime_error("Builder: Unknown encoder type");
        }
        msg_w.pass();
        return "";
    }

private:
    json payload_;
};


#endif  // __M2E_BRIDGE_BUILDER_FT_H__