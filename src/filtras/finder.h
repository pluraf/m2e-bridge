/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_BRIDGE_FINDER_FT_H__
#define __M2E_BRIDGE_FINDER_FT_H__


#include <variant>

#include "filtra.h"


enum class SearchOperator {UNKN, CONTAIN, CONTAINED, MATCH};


class FinderFT:public Filtra{
public:
    FinderFT(PipelineIface const & pi, json const & config):Filtra(pi, config){
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

    string process_message(MessageWrapper & msg_w)override{
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
            msg_w.pass();
        }else{
            msg_w.reject();
        }
        return "";
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
        if(msg_format_ == MessageFormat::JSON){
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
        if(msg_format_ == MessageFormat::JSON){
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


#endif  // __M2E_BRIDGE_FINDER_FT_H__