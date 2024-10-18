#ifndef __M2E_BRIDGE_SEARCH_FILTER_H__
#define __M2E_BRIDGE_SEARCH_FILTER_H__


#include <variant>

#include "filtra.h"


enum class SearchOperator {UNKN, CONTAIN, CONTAINED, MATCH};


class SearchFilter:public Filtra{
public:
    SearchFilter(PipelineIface const & pi, json const & config):Filtra(pi, config){
        std::string const & oper = config.value("operator", "match");
        if(oper == "contain"){
            operator_ = SearchOperator::CONTAIN;
        }else if (oper == "contained"){
            operator_ = SearchOperator::CONTAINED;
        }else if (oper == "match"){
            operator_ = SearchOperator::MATCH;
        }

        if(config.contains("string")){
            string_ = config["string"];
        }

        if(config.contains("keys")){
            json j_keys = config["keys"];
            keys_ = vector<string>(j_keys.begin(), j_keys.end());
        }

        if(config.contains("value_key")){
            value_key_ = config["value_key"];
        }
    }

    void pass(MessageWrapper & msg_w)override{
        bool res = true;
        if(string_.size() > 0){
            if(value_key_.size() > 0){
                res &= find_in_value(msg_w);
            }else{
                res &= find_in_string(msg_w.msg().get_raw());
            }
        }
        if(res && keys_.size() > 0){
            res &= find_in_keys(msg_w);
        }
        res = logical_negation_ ? ! res : res;
        if(res){
            msg_w.accept();
        }else{
            msg_w.reject();
        }
    }

    bool find_in_string(string const & msg_string){
        if(operator_ == SearchOperator::CONTAIN){
            return msg_string.find(string_) != std::string::npos;
        }else if(operator_ == SearchOperator::CONTAINED){
            return string_.find(msg_string) != std::string::npos;
        }else if(operator_ == SearchOperator::MATCH){
            return string_ == msg_string;
        }else{
            return false;
        }
    }

    bool find_in_keys(MessageWrapper & msg_w){
        if(decoder_ == MessageFormat::JSON){
            json const & j_payload = msg_w.msg().get_json();
            for(auto const & key : keys_){
                if(! j_payload.contains(key)){
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    bool find_in_value(MessageWrapper & msg_w){
        if(decoder_ == MessageFormat::JSON){
            json const & j_payload = msg_w.msg().get_json();
            if(j_payload.contains(value_key_)){
                return find_in_string(j_payload[value_key_]);
            }else{
                return false;
            }
        }
        return false;
    }

private:
    SearchOperator operator_ {SearchOperator::UNKN};
    std::string string_;
    vector<string> keys_;
    std::string value_key_;
};


#endif  // __M2E_BRIDGE_SEARCH_FILTER_H__