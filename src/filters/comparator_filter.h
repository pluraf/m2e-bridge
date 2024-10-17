#ifndef __M2E_BRIDGE_COMPARATOR_FILTER_H__
#define __M2E_BRIDGE_COMPARATOR_FILTER_H__


#include <string>
#include <variant>

#include "filter.h"


enum class ComparatorOperator {UNKN, EQ, GT, GTE, LT, LTE};


class ComparatorFilter:public Filter{
public:
    ComparatorFilter(json const & json_descr):Filter(json_descr){
        std::string const & oper = json_descr["operator"];
        if(oper == "eq"){
            operator_ = ComparatorOperator::EQ;
        }else if (oper == "gt"){
            operator_ = ComparatorOperator::GT;
        }else if (oper == "gte"){
            operator_ = ComparatorOperator::GTE;
        }else if (oper == "lt"){
            operator_ = ComparatorOperator::LT;
        }else if (oper == "lte"){
            operator_ = ComparatorOperator::LTE;
        }

        value_key_ = json_descr["value_key"];

        json const & j_comparand = json_descr.at("comparand");
        if(j_comparand.is_number_float()){
            comparand_ = j_comparand.get<double>();
        }else if(j_comparand.is_number_integer()){
            comparand_ = j_comparand.get<long long>();
        }
    }

    bool pass(MessageWrapper & msg_w){
        json const & payload = msg_w.orig().get_json();
        bool res = false;
        try{
            json const & j_value = payload.at(value_key_);
            if(j_value.is_number_float() || std::holds_alternative<double>(comparand_)){
                res = compare(j_value.get<double>(), std::get<double>(comparand_));
            }else if(j_value.is_number_integer()){
                res = compare(j_value.get<long long>(), std::get<long long>(comparand_));
            }else{
                return false;
            }
            return logical_negation_ ? ! res : res;
        }catch(json::exception){
            return false;
        }
    }
private:
    ComparatorOperator operator_ {ComparatorOperator::UNKN};
    std::string value_key_;
    std::variant<long long, double> comparand_;

    template <typename T>
    bool compare(T a, T b){
        switch(operator_){
            case ComparatorOperator::EQ: return a == b;
            case ComparatorOperator::GT: return a > b;
            case ComparatorOperator::GTE: return a >= b;
            case ComparatorOperator::LT: return a < b;
            case ComparatorOperator::LTE: return a <= b;
            default: return false;
        }
    }
};


#endif  // __M2E_BRIDGE_COMPARATOR_FILTER_H__