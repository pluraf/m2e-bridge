#ifndef __M2E_BRIDGE_NOP_FT_H__
#define __M2E_BRIDGE_NOP_FT_H__


#include "filtra.h"


class NopFT:public Filtra{
public:
    NopFT(PipelineIface const & pi, json const & json_descr):Filtra(pi, json_descr){}
    string process_message(MessageWrapper &msg_w)override{
        msg_w.pass();
        return "";
    }
};


#endif  // __M2E_BRIDGE_NOP_FT_H__