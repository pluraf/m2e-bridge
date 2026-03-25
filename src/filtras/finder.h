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


namespace
{
    //_DOCS: STRINGS_START
    constexpr std::string_view _DOCS_PR_DESC_TEXT =
        "Specifies a string that can either be searched as a substring within"
        " the payload or used to search for the payload as a substring.";

    constexpr std::string_view _DOCS_PR_DESC_KEYS =
        "Specifies the keys in the decoded payload dictionary. All keys must"
        " be present in the payload; otherwise, the message is rejected.";

    constexpr std::string_view _DOCS_PR_DESC_VALUE_KEY =
        "Specifies a key in the decoded payload whose value is used"
        " for the text search.";
    //_DOCS: END
}


//_DOCS: SECTION_START finder_filtra Finder Filtra
/*!
Searches for a specific part in the message based on the finder configuration.
Three variants are supported:

.. __:

1. Searches for text within the payload or searches for the payload within the text.
   Properties *text* and *operator* must be specified::

      {
          "type": "finder",
          "operator": "contain",
          "text": "LOG"
      }

2. Searches for specified keys in the decoded payload. A property *keys* must be specified.

   ::

      {
          "type": "finder",
          "keys": ["key1", "keyN"]
      }

3. Performs the same type of search as in `Variant 1`__, but instead of analyzing
   the entire payload, it examines the value of a specified key in
   the decoded payload. Properties *text*, *operator*, *value_key* must be specified::

      {
          "type": "finder",
          "operator": "contain",
          "text": "LOG",
          "value_key": "key1"
      }
*/
//_DOCS: END

class FinderFT: public Filtra
{
public:
    FinderFT(PipelineIface const & pi, json const & config): Filtra(pi, config)
    {
        std::string const & oper = config.value("operator", "match");

        if(oper == "contain"){
            operator_ = SearchOperator::CONTAIN;
        }else if (oper == "contained"){
            operator_ = SearchOperator::CONTAINED;
        }else if (oper == "match"){
            operator_ = SearchOperator::MATCH;
        }

        if(config.contains("text")){
            text_ = config["text"];
        }

        if(config.contains("keys")){
            json j_keys = config["keys"];
            keys_ = vector<string>(j_keys.begin(), j_keys.end());
        }

        if(config.contains("value_key")){
            value_key_ = config["value_key"];
        }
    }

    string process_message(MessageWrapper & msg_w) override
    {
        bool res{ true };
        bool checked{ false };

        if( res && text_.size() > 0 )
        {
            checked = true;
            if(value_key_.size() > 0){
                res &= find_in_value(msg_w);
            }else{
                res &= find_in_string(msg_w.msg().get_raw());
            }
        }
        if(res && keys_.size() > 0){
            checked = true;
            res &= find_in_keys(msg_w);
        }
        res = checked && res;
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
            return msg_string.find(text_) != std::string::npos;
        }else if(operator_ == SearchOperator::CONTAINED){
            return text_.find(msg_string) != std::string::npos;
        }else if(operator_ == SearchOperator::MATCH){
            return text_ == msg_string;
        }else{
            return false;
        }
    }

    bool find_in_keys(MessageWrapper & msg_w){
        if(msg_format_ == MessageFormat::Type::JSON){
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
        if(msg_format_ == MessageFormat::Type::JSON){
            json const & j_payload = msg_w.msg().get_json();
            if(j_payload.contains(value_key_)){
                return find_in_string(j_payload[value_key_]);
            }else{
                return false;
            }
        }
        return false;
    }

    static pair<string, json> get_schema(){
        //_DOCS: SCHEMA_START finder_filtra
        //_DOCS: SCHEMA_INCLUDE filtra
        static json schema = Filtra::get_schema({
            {"tags", {"finder"}},
            {"type_properties", {
                {"operator", {
                    {"type", "string"},
                    {"options", json::array_t{
                        {"contain", "searches for *text* within the payload"},
                        {"contained", "searches for the payload within *text*"},
                        {"match", "checks if *text* exactly matches the payload"}
                    }},
                    {"default", "match"},
                    {"required", true},
                    {"description", "Method for searching for a substring."}
                }},
                {"text", {
                    {"type", "string"},
                    {"required", false},
                    {"description", _DOCS_PR_DESC_TEXT}
                }},
                {"keys", {
                    {"type", "array"},
                    {"items", {{"type", "string"}}},
                    {"required", false},
                    {"description", _DOCS_PR_DESC_KEYS}
                }},
                {"value_key", {
                    {"type", "string"},
                    {"required", false},
                    {"description", _DOCS_PR_DESC_VALUE_KEY}
                }},
                {"decoder", {
                    {"type", "string"},
                    {"options", {"json", "raw"}},
                    {"default", "raw"},
                    {"required", false},
                    {"description", "Decoder for decoding the message."}
                }}
            }}
        });
        //_DOCS: END
        return {"finder", schema};
    }

private:
    SearchOperator operator_ {SearchOperator::UNKN};
    std::string text_;
    vector<string> keys_;
    std::string value_key_;
};


#endif  // __M2E_BRIDGE_FINDER_FT_H__