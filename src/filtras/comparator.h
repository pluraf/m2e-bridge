#ifndef __M2E_BRIDGE_COMPARATOR_FT_H__
#define __M2E_BRIDGE_COMPARATOR_FT_H__


#include <variant>

#include "filtra.h"


enum class ComparatorOperator {UNKN, EQ, GT, GTE, LT, LTE};


class ComparatorFT:public Filtra{
public:
    ComparatorFT(PipelineIface const & pi, json const & json_descr):Filtra(pi, json_descr){
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

    void process(MessageWrapper & msg_w)override{
        json const & payload = msg_w.msg().get_json();
        bool res = false;
        try{
            json const & j_value = payload.at(value_key_);
            if(j_value.is_number_float()){
                if(std::holds_alternative<double>(comparand_)){
                    res = compare(j_value.get<double>(), std::get<double>(comparand_));
                }else{
                    res = compare(j_value.get<double>(), std::get<long long>(comparand_));
                }
            }else if(j_value.is_number_integer()){
                if(std::holds_alternative<double>(comparand_)){
                    res = compare(j_value.get<long long>(), std::get<double>(comparand_));
                }else{
                    res = compare(j_value.get<long long>(), std::get<long long>(comparand_));
                }
            }else{
                throw std::invalid_argument("json");
            }
            res = logical_negation_ ? ! res : res;
            if(res){
                msg_w.pass();
            }else{
                msg_w.reject();
            }
        }catch(json::exception){
            throw std::invalid_argument("json");
        }
    }
private:
    ComparatorOperator operator_ {ComparatorOperator::UNKN};
    std::string value_key_;
    std::variant<long long, double> comparand_;

    template <typename A, typename B>
    bool compare(A a, B b){
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


#endif  // __M2E_BRIDGE_COMPARATOR_FT_H__