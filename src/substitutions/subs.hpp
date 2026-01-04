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


#ifndef __M2E_BRIDGE_SUBS_H__
#define __M2E_BRIDGE_SUBS_H__


#include <regex>
#include <string>
#include <variant>
#include <span>

#include <fmt/core.h>

#include "m2e_aliases.h"
#include "shared_objects.h"
#include "m2e_message/message_wrapper.h"


using StringMap = std::map<std::string, std::string>;
using EnvObjects = std::map<std::string, std::variant<Message *, json const *, StringMap const *, MessageExtra const *>>;

using substituted_t = std::variant<string, nlohmann::json, std::span<const std::byte>, std::vector<unsigned char>>;


class SubsEngine {
    EnvObjects env_;
    substituted_t evaluate(string const & exptrssion);
public:
    SubsEngine(Message & msg, json const & meta, json const & attr){
        env_["MSG"] = & msg;
        env_["META"] = & meta;
        env_["ATTR"] = & attr;
        // For now, only JSON shared objects are supported
        env_["SHARED"] = & SharedObjects::get_json();
    }

    SubsEngine(Message & msg, json const & meta, StringMap const & attr){
        env_["MSG"] = & msg;
        env_["META"] = & meta;
        env_["ATTR"] = & attr;
        // For now, only JSON shared objects are supported
        env_["SHARED"] = & SharedObjects::get_json();
    }

    SubsEngine(MessageWrapper & msg_w){
        env_["MSG"] = & msg_w.msg();
        env_["META"] = & msg_w.get_metadata();
        env_["ATTR"] = & msg_w.msg().get_attributes();
        env_["EXTRA"] = & msg_w.get_extra();
        // For now, only JSON shared objects are supported
        env_["SHARED"] = & SharedObjects::get_json();
    }

    substituted_t substitute(string const & atemplate){
        using namespace std;
        regex pattern("\\{\\{(.*?)\\}\\}");
        smatch match;
        string result = atemplate;
        try{
            auto pos = result.cbegin();
            while(regex_search(pos, result.cend(), match, pattern)){
                string expression = match[1].str();
                if(match[0].str() == atemplate){
                    return evaluate(expression);
                }else{
                    try{
                        string vvalue {std::get<string>(evaluate(expression))};
                        unsigned int i = (pos - result.cbegin());
                        result.replace(i + match.position(), match.length(), vvalue);
                        // Restore iterator after string modification
                        pos = result.cbegin() + i + match.position() + vvalue.size();
                    }catch(json::exception){
                        throw runtime_error(fmt::format("Can not evaluate: {}!", expression));
                    }
                }
            }
        }catch(json::exception const e){
            throw runtime_error(e.what());
        }
        return result;
    }
};


#endif  // __M2E_BRIDGE_SUBS_H__