#ifndef __M2E_BRIDGE_ERASER_TRANSFORMER_H__
#define __M2E_BRIDGE_ERASER_TRANSFORMER_H__


#include "filtra.h"


class EraserTransformer:public Filtra{
public:
    EraserTransformer(PipelineIface const & pi, json const & json_descr):
            Filtra(pi, json_descr){
        json const & j_keys = json_descr["keys"];
        keys_ = vector<string>(j_keys.begin(), j_keys.end());
    }

    void pass(MessageWrapper &msg_w)override{
        if(decoder_ == MessageFormat::JSON){
            json & j_payload = msg_w.msg().get_json();
            for(auto const & key : keys_){
                try{
                    j_payload.erase(key);
                }catch(std::exception){
                    continue;
                }
            }
        }
    }

private:
    vector<string> keys_;
};


#endif  // __M2E_BRIDGE_ERASER_TRANSFORMER_H__