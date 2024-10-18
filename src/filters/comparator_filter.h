#ifndef __M2E_BRIDGE_COMPARATOR_FILTER_H__
#define __M2E_BRIDGE_COMPARATOR_FILTER_H__


#include <variant>

#include "filtra.h"


enum class ComparatorOperator {UNKN, EQ, GT, GTE, LT, LTE};


class ComparatorFilter:public Filtra{
public:
    ComparatorFilter(PipelineIface const & pi, json const & json_descr):Filtra(pi, json_descr){
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

    void pass(MessageWrapper & msg_w)override{
        json const & payload = msg_w.msg().get_json();
        bool res = false;
        try{
            json const & j_value = payload.at(value_key_);
            if(j_value.is_number_float() || std::holds_alternative<double>(comparand_)){
                res = compare(j_value.get<double>(), std::get<double>(comparand_));
            }else if(j_value.is_number_integer()){
                res = compare(j_value.get<long long>(), std::get<long long>(comparand_));
            }else{
                msg_w.reject();
                return;
            }
            res = logical_negation_ ? ! res : res;
            if(res){
                msg_w.accept();
            }else{
                msg_w.reject();
            }
        }catch(json::exception){
            msg_w.reject();
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