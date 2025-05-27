/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2025 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_BRIDGE_SUBS_H__
#define __M2E_BRIDGE_SUBS_H__


#include <regex>
#include <string>
#include <variant>

#include <fmt/core.h>

#include "m2e_aliases.h"
#include "m2e_message/message.h"


using StringMap = std::map<std::string, std::string>;
using EnvObjects = std::map<std::string, std::variant<Message *, json const *, StringMap const *>>;


class SubsEngine {
    EnvObjects env_;
public:
    SubsEngine(Message & msg, json const & meta, json const & attr){
        env_["MSG"] = & msg;
        env_["ATTR"] = & attr;
        env_["META"] = & meta;
    }

    SubsEngine(Message & msg, json const & meta, StringMap const & attr){
        env_["MSG"] = & msg;
        env_["ATTR"] = & attr;
        env_["META"] = & meta;
    }

    std::variant<string, nlohmann::json> substitute(string const & atemplate){
        using namespace std;
        regex pattern("\\{\\{(.*?)\\}\\}");
        smatch match;
        string result = atemplate;
        try{
            auto pos = result.cbegin();
            while(regex_search(pos, result.cend(), match, pattern)){
                string expression = match[1].str();
                if(match[0].str() == atemplate){
                    return evaluate<nlohmann::json>(expression);
                }else{
                    try{
                        string vvalue = evaluate<string>(expression);
                        unsigned int i = (pos - result.cbegin());
                        result.replace(i + match.position(), match.length(), vvalue);
                        // Restore iterator after string modification
                        pos = result.cbegin() + i + match.position() + vvalue.size();
                    }catch(json::exception){
                        throw runtime_error(fmt::format("Can not evaluate: {}!", expression));
                    }
                }
            }
        }catch(json::exception){
            throw runtime_error("Message payload is not a valid JSON!");
        }
        return result;
    }

    template<typename T>
    T evaluate(string const & exptrssion);
};


#endif  // __M2E_BRIDGE_SUBS_H__