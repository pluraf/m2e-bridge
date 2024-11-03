#ifndef __M2E_BRIDGE_FILTRA_H__
#define __M2E_BRIDGE_FILTRA_H__


#include "m2e_aliases.h"
#include "pipeline_iface.h"
#include "m2e_message/message_wrapper.h"
#include "iostream"


class Filtra{
public:
    Filtra(PipelineIface const & pipeline, json const & config):name_(){
        name_ = config.value("name", "");

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

        if(config.contains("metadata")){
            metadata_ = config.at("metadata");
        }

        string goto_hop = config.value("goto", "");
        hops_.first = config.value("goto_passed", goto_hop);
        hops_.second = config.value("goto_rejected", goto_hop);
    }
    virtual ~Filtra(){}
    string process(MessageWrapper & msg_w){
        msg_w.add_metadata(metadata_);
        return process_message(msg_w);
    }
    Message process(){
        return generate_message();
    }
    vector<string> const & get_destinations(){return queue_ids_;}
    string const & get_name(){return name_;}
    hops_t const & get_hops(){return hops_;}

protected:
    virtual string process_message(MessageWrapper &msg_w) = 0;
    virtual Message generate_message(){return Message();}

    MessageFormat decoder_ {MessageFormat::UNKN};
    MessageFormat encoder_ {MessageFormat::UNKN};
    bool logical_negation_ {false};
    vector<string> queue_ids_;

private:
    string name_;
    hops_t hops_;
    json metadata_;
};


#endif  // __M2E_BRIDGE_FILTRA_H__
