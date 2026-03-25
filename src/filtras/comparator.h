/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024-2026 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


#ifndef __M2E_BRIDGE_COMPARATOR_FT_H__
#define __M2E_BRIDGE_COMPARATOR_FT_H__


#include <variant>

#include "filtra.h"


enum class ComparatorOperator {UNKN, EQ, GT, GTE, LT, LTE};


//_DOCS: SECTION_START comparator_filtra Comparator Filtra
/*!
Creates a new message::

    {
      "type": "comparator",
      "operator": "gt",
      "value_key": "temp",
      "comparand": 5.4
    }

*/
//_DOCS: END

class ComparatorFT: public Filtra
{
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

    string process_message(MessageWrapper & msg_w)override{
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
        return "";
    }

    static pair<string, json> get_schema(){
        //_DOCS: SCHEMA_START comparator_filtra
        //_DOCS: SCHEMA_INCLUDE filtra
        static json schema = Filtra::get_schema(
            {
                {"tags", {"comparator"}},
                {"type_properties", {
                    {"operator", {
                        {"type", "string"},
                        {"options", json::array_t{
                            {"eq", "equal"},
                            {"gt", "greater than"},
                            {"gte", "greater than or equal"},
                            {"lt", "less than"},
                            {"lte", "less than or equal"}
                        }},
                        {"required", true},
                        {"description", "Comparison operator."}
                    }},
                    {"value_key", {
                        {"type", "string"},
                        {"required", true},
                        {"description", "Key in the payload whose value is compared with the comparand."}
                    }},
                    {"comparand", {
                        {"type", "float"},
                        {"required", true},
                        {"description", "Numerical constant to be compared with a value in the payload."}
                    }}
                }}
            }
        );
        //_DOCS: END
        return {"comparator", schema};
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