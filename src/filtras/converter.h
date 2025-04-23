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


#ifndef __M2E_BRIDGE_CONVERTER_FT_H__
#define __M2E_BRIDGE_CONVERTER_FT_H__


#include <lua.hpp>

#include "filtra.h"
#include "database/converter.h"

extern "C" {
    extern int luaopen_cjson(lua_State *L);
}


class ConverterFT: public Filtra{
    string converter_id_;
    string lua_code_;
    lua_State * L_ = nullptr;

    void load_lua_code(){
        ConverterTable ct;
        Converter c;
        bool res = ct.get(converter_id_, c);
        if(res){
            lua_code_ = c.code;
            L_ = luaL_newstate();
            luaL_requiref(L_, "base", luaopen_base, 1);
            luaL_requiref(L_, "string", luaopen_string, 1);
            luaL_requiref(L_, "utf8", luaopen_utf8, 1);
            luaL_requiref(L_, "table", luaopen_table, 1);
            luaL_requiref(L_, "math", luaopen_math, 1);
            luaL_requiref(L_, "cjson", luaopen_cjson, 1);
            lua_pop(L_, 6);  // Must be equal to number of luaL_requiref calls!
            if(luaL_dostring(L_, lua_code_.c_str())){
                throw std::runtime_error(lua_tostring(L_, -1));
            }
        }else{
            throw std::runtime_error("Not able to retreive converter code!");
        }
    }
public:
    ConverterFT(PipelineIface const & pi, json const & config):
            Filtra(pi, config){
        converter_id_ = config.at("converter_id").get<string>();
    }

    void start()override{
        if(L_) lua_close(L_);
        load_lua_code();
    }

    string process_message(MessageWrapper &msg_w)override{
        Message msg = msg_w.orig();
        string raw = msg.get_raw();

        lua_getglobal(L_, "convert");
        lua_pushlstring(L_, raw.c_str(), 2);
        if(lua_pcall(L_, 1, 1, 0)){
            throw std::runtime_error(lua_tostring(L_, -1));
        }
        msg_w.msg().get_raw() =  lua_tostring(L_, -1);
        msg_w.pass();
        return "";
    }

    static pair<string, json> get_schema(){
        json schema = Filtra::get_schema();
        schema.merge_patch({
            {"converter_id", {
                {"type", "string"},
                {"options", {
                    {"url", "api/converter/"},
                    {"key", "converter_id"},
                }},
                {"required", true}
            }}
        });
        return {"converter", schema};
    }
};


#endif  // __M2E_BRIDGE_CONVERTER_FT_H__