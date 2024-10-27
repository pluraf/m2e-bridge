#ifndef __M2E_BRIDGE_FILTRA_H__
#define __M2E_BRIDGE_FILTRA_H__


#include "m2e_aliases.h"
#include "pipeline_iface.h"
#include "m2e_message/message_wrapper.h"
#include "iostream"


class Filtra{
public:
    Filtra(PipelineIface const & pipeline, json const & config):name_(){
        std::string const & decoder = config.value("decoder", "raw");
        if(decoder == "json"){
            decoder_ = MessageFormat::JSON;
        }else if(decoder == "raw"){
            decoder_ = MessageFormat::RAW;
        }

        std::string const & encoder = config.value("encoder", "raw");
        if(encoder == "json"){
            encoder_ = MessageFormat::JSON;
        }else if(encoder == "raw"){
            encoder_ = MessageFormat::RAW;
        }

        logical_negation_ = config.value("logical_negation", false);

        if(config.contains("queues")){
            json const & j_queues = config.at("queues");
            queue_ids_ = vector<string>(j_queues.begin(), j_queues.end());
        }

        name_ = config.value("name", "");

        hops_.first = config.value("goto_passed", "");

        hops_.second = config.value("goto_rejected", "");
    }
    virtual ~Filtra(){}
    virtual string process(MessageWrapper &msg_w) = 0;
    virtual Message process(){return Message();}
    vector<string> const & get_destinations(){return queue_ids_;}
    string const & get_name(){return name_;}
    hops_t const & get_hops(){return hops_;}

protected:
    MessageFormat decoder_ {MessageFormat::UNKN};
    MessageFormat encoder_ {MessageFormat::UNKN};
    bool logical_negation_ {false};
    vector<string> queue_ids_;

private:
    string name_;
    hops_t hops_;
};


#endif  // __M2E_BRIDGE_FILTRA_H__
