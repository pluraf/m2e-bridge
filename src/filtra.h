#ifndef __M2E_BRIDGE_FILTRA_H__
#define __M2E_BRIDGE_FILTRA_H__


#include "m2e_aliases.h"
#include "pipeline_iface.h"
#include "m2e_message/message_wrapper.h"
#include "iostream"

class Filtra{
public:
    Filtra(PipelineIface const & pipeline, json const & json_descr){
        std::string const & decoder = json_descr.value("decoder", "raw");
        if(decoder == "json"){
            decoder_ = MessageFormat::JSON;
        }else if(decoder == "raw"){
            decoder_ = MessageFormat::RAW;
        }
        logical_negation_ = json_descr.value("logical_negation", false);
    }
    virtual ~Filtra(){}
    virtual void pass(MessageWrapper &msg_w) = 0;
    virtual MessageWrapper pass(){return MessageWrapper();}
protected:
    MessageFormat decoder_ {MessageFormat::UNKN};
    bool logical_negation_ {false};
};


#endif  // __M2E_BRIDGE_FILTRA_H__
