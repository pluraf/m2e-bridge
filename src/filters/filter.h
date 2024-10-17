#ifndef __M2E_BRIDGE_FILTER_H__
#define __M2E_BRIDGE_FILTER_H__


#include <string>

#include "m2e_aliases.h"
#include "m2e_message/message_wrapper.h"


enum class PayloadDecoder {UNKN, TEXT, JSON};


class Filter {
public:
    Filter(json const & json_descr){
        std::string const & decoder = json_descr["decoder"];
        if(decoder == "json"){
            decoder_ = PayloadDecoder::JSON;
        }else if (decoder == "text"){
            decoder_ = PayloadDecoder::TEXT;
        }
        bool logical_negation_ = json_descr.value("logical_negation", false);
    }

    bool pass(const MessageWrapper &msg_w) { return true; }
protected:
    PayloadDecoder decoder_ {PayloadDecoder::UNKN};
    bool logical_negation_ {false};
};


#endif  // __M2E_BRIDGE_FILTER_H__