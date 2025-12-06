/* SPDX-License-Identifier: BSD-3-Clause */

/*
Copyright (c) 2024-2025 Pluraf Embedded AB <code@pluraf.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include "m2e_message/message.h"


using StringMap = std::map<std::string, std::string>;
using EnvObjects = std::map<std::string, std::variant<Message *, json const *, StringMap const *>>;

using substituted_t = std::variant<string, nlohmann::json, std::span<std::byte>>;


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