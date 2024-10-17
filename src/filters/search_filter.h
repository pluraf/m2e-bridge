#ifndef __M2E_BRIDGE_SEARCH_FILTER_H__
#define __M2E_BRIDGE_SEARCH_FILTER_H__


#include <string>
#include <variant>

#include "filter.h"


enum class SearchOperator {UNKN, CONTAIN, CONTAINED};


class SearchFilter:public Filter{
public:
    SearchFilter(json const & json_descr):Filter(json_descr){
        std::string const & oper = json_descr["operator"];
        if(oper == "contain"){
            operator_ = SearchOperator::CONTAIN;
        }else if (oper == "contained"){
            operator_ = SearchOperator::CONTAINED;
        }

        string_ = json_descr["string"];
    }

    bool pass(MessageWrapper & msg_w){
        std::string const & payload = msg_w.get_text();
        bool res = false;
        if(operator_ == SearchOperator::CONTAIN){
            res = payload.find(string_) != std::string::npos;
        }else if(operator_ == SearchOperator::CONTAINED){
            res = string_.find(payload) != std::string::npos;
        }else{
            return false;
        }
        return logical_negation_ ? ! res : res;
    }
private:
    SearchOperator operator_ {SearchOperator::UNKN};
    std::string string_;
};


#endif  // __M2E_BRIDGE_SEARCH_FILTER_H__