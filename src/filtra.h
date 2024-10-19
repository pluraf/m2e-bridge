#ifndef __M2E_BRIDGE_FILTRA_H__
#define __M2E_BRIDGE_FILTRA_H__


#include "m2e_aliases.h"
#include "pipeline_iface.h"
#include "m2e_message/message_wrapper.h"
#include "iostream"

class Filtra{
public:
    Filtra(PipelineIface const & pipeline, json const & config){
        std::string const & decoder = config.value("decoder", "raw");
        if(decoder == "json"){
            decoder_ = MessageFormat::JSON;
        }else if(decoder == "raw"){
            decoder_ = MessageFormat::RAW;
        }
        logical_negation_ = config.value("logical_negation", false);
        if(config.contains("queues")){
            json const & j_queues = config.at("queues");
            queue_ids_ = vector<string>(j_queues.begin(), j_queues.end());
        }
    }
    virtual ~Filtra(){}
    virtual void pass(MessageWrapper &msg_w) = 0;
    virtual MessageWrapper pass(){return MessageWrapper();}
    vector<string> const & get_destinations(){return queue_ids_;}
protected:
    MessageFormat decoder_ {MessageFormat::UNKN};
    bool logical_negation_ {false};
    vector<string> queue_ids_;
};


#endif  // __M2E_BRIDGE_FILTRA_H__
