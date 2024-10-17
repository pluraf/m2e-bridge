#ifndef __M2E_BRIDGE_REDUCTION_TRANSFORMER_H__
#define __M2E_BRIDGE_REDUCTION_TRANSFORMER_H__


#include "transformer.h"

#include <string>
#include <vector>


class EraserTransformer:public Transformer{
public:
    EraserTransformer(json const & json_descr):Transformer(json_descr){
        json const & j_keys = json_descr["keys"];
        keys_ = vector(j_keys.begin, j_keys.end());
    }

    void pass(MessageWrapper &msg_w){
        json & j_payload = msg_w.alt().get_json();
        for(auto const & key : keys_){
            try{
                j_payload.erase(key);
            }catch(son::exception){
                continue;
            }
        }
    }
private:
    vector<string> keys_;
};


#endif  // __M2E_BRIDGE_REDUCTION_TRANSFORMER_H__