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


#include <tao/pegtl.hpp>
#include <cbor.h>

#include "modifiers/modifier.h"

#include "subs.hpp"


class ObjectProxy
{
    std::variant<
        Message *,
        nlohmann::json const *,
        std::vector<std::string> const *,
        std::string const *,
        std::map<std::string, std::string> const *,
        cbor_item_t const *,
        MessageExtra const *
    > obj_;
public:
    ObjectProxy() = default;

    ObjectProxy(Message * msg){
        obj_ = msg;
    }

    ObjectProxy(nlohmann::json const * j){
        obj_ = j;
    }

    ObjectProxy(cbor_item_t const * c){
        obj_ = c;
    }

    ObjectProxy(std::vector<std::string> const * v){
        obj_ = v;
    }

    ObjectProxy(std::string const * s){
        obj_ = s;
    }

    ObjectProxy(std::map<std::string, std::string> const * m){
        obj_ = m;
    }

    ObjectProxy(MessageExtra const * extra){
        obj_ = extra;
    }

    ObjectProxy next(std::string const & key){
        if(std::holds_alternative<Message *>(obj_)){
            if(key == "PAYLOAD"){
                Message * msg = std::get<Message *>(obj_);
                switch(msg->get_format()){
                    case MessageFormat::Type::JSON:
                        return ObjectProxy(&std::get<Message *>(obj_)->get_json());
                    case MessageFormat::Type::CBOR:
                        return ObjectProxy(std::get<Message *>(obj_)->get_cbor());
                }
            }
            else if(key == "TOPIC_LEVELS"){
                return ObjectProxy(& std::get<Message *>(obj_)->get_topic_levels());
            }
            else if(key == "TOPIC"){
                return ObjectProxy(& std::get<Message *>(obj_)->get_topic());
            }
        }
        else if(std::holds_alternative<nlohmann::json const *>(obj_)){
            return ObjectProxy(&std::get<nlohmann::json const *>(obj_)->at(key));
        }
        else if(std::holds_alternative<std::map<std::string, std::string> const *>(obj_)){
            return ObjectProxy(&std::get<std::map<std::string, std::string> const *>(obj_)->at(key));
        }
        else if(std::holds_alternative<cbor_item_t const *>(obj_)){
            cbor_item_t const * cbor_map = std::get<cbor_item_t const *>(obj_);
            struct cbor_pair *pairs = cbor_map_handle(cbor_map);
            size_t map_size = cbor_map_size(cbor_map);
            for (size_t i = 0; i < map_size; ++i){
                char *map_key = (char *)cbor_string_handle(pairs[i].key);
                size_t map_key_len = cbor_string_length(pairs[i].key);
                if(key.compare(0, key.size(), map_key, map_key_len) == 0){
                    return ObjectProxy(pairs[i].value);
                }
            }
        }
        else if(std::holds_alternative<MessageExtra const *>(obj_)){
            auto extra = std::get<MessageExtra const *>(obj_);
            extra->set_key(key);
            return ObjectProxy(extra);
        }

        throw std::out_of_range(fmt::format("Can not find key: {}!", key));
    }

    ObjectProxy next(unsigned ix){
        if(std::holds_alternative<std::vector<std::string> const *>(obj_)){
            return ObjectProxy(& std::get<std::vector<std::string> const *>(obj_)->at(ix));
        }else if(std::holds_alternative<nlohmann::json const *>(obj_)){
            return ObjectProxy(& std::get<nlohmann::json const *>(obj_)->at(ix));
        }
        throw std::invalid_argument("Index access is not available!");
    }

    substituted_t value(){
        if(std::holds_alternative<std::string const *>(obj_)){
            return *std::get<std::string const *>(obj_);
        }
        else if(std::holds_alternative<nlohmann::json const *>(obj_)){
            nlohmann::json const *j = std::get<nlohmann::json const *>(obj_);
            if(j->is_string()){
                return std::get<nlohmann::json const *>(obj_)->get<std::string>();
            }
            else if(j->is_number_integer()){
                return std::to_string(std::get<nlohmann::json const *>(obj_)->template get<unsigned>());
            }
            else{
                return *std::get<nlohmann::json const *>(obj_);
            }
        }
        else if(std::holds_alternative<cbor_item_t const *>(obj_)){
            cbor_item_t const * item = std::get<cbor_item_t const *>(obj_);
            if(cbor_is_int(item)){
                cbor_int_width width = cbor_int_get_width(item);
                switch(width){
                    case CBOR_INT_8: return static_cast<long>(cbor_get_uint8(item));
                    case CBOR_INT_16: return static_cast<long>(cbor_get_uint16(item));
                    case CBOR_INT_32: return static_cast<long>(cbor_get_uint32(item));
                    case CBOR_INT_64: return static_cast<long>(cbor_get_uint64(item));
                }
            }
            if(cbor_is_float(item)){
                cbor_float_width width = cbor_float_get_width(item);
                switch(width){
                    case CBOR_FLOAT_16: return static_cast<double>(cbor_float_get_float2(item));
                    case CBOR_FLOAT_32: return static_cast<double>(cbor_float_get_float4(item));
                    case CBOR_FLOAT_64: return static_cast<double>(cbor_float_get_float8(item));
                }
            }
            else if(cbor_isa_bytestring(item)){
                return std::span<byte const>(
                    reinterpret_cast<byte *>(cbor_bytestring_handle(item)),
                    cbor_bytestring_length(item)
                );
            }
            else if(cbor_isa_string(item)){
                return std::string(
                    reinterpret_cast<char *>(cbor_string_handle(item)),
                    reinterpret_cast<char *>(cbor_string_handle(item) + cbor_string_length(item))
                );
            }
        }
        else if(std::holds_alternative<MessageExtra const *>(obj_)){
            auto extra = std::get<MessageExtra const *>(obj_);
            return std::span<byte const>(extra->get_extra(), extra->get_extra_size());
        }

        throw std::runtime_error("Value is not substitutable!");
    }
};


namespace grammar {
    using namespace tao::pegtl;

    struct dot : tao::pegtl::string<'.'> {};
    struct comma : tao::pegtl::string<','> {};
    struct pipe : tao::pegtl::string<'|'> {};
    struct lbracket : tao::pegtl::string<'['> {};
    struct rbracket : tao::pegtl::string<']'> {};
    struct modifier : seq<pipe, identifier_first, star<identifier_other>, opt<comma, plus<digit>>> {};
    struct property : seq<dot, identifier_first, star<identifier_other>> {};
    struct index : seq<lbracket, plus<digit>, rbracket> {};
    struct expression : seq<identifier, plus<sor<property, index>>, opt<modifier>> {};
}


class EvalState {
    EnvObjects & env_;
    ObjectProxy obj_;
    std::vector<unsigned char> value_;
public:
    EvalState( EnvObjects & env ) :env_(env) {}

    void start(std::string const & name){
        auto v = env_.at(name);
        if( std::holds_alternative<Message *>(v) )
        {
            obj_ = ObjectProxy(std::get<Message *>(v));
        }
        else if( std::holds_alternative<json const *>(v) )
        {
            obj_ = ObjectProxy(std::get<json const *>(v));
        }
        else if( std::holds_alternative<StringMap const *>(v) )
        {
            obj_ = ObjectProxy(std::get<StringMap const *>(v));
        }
        else if( std::holds_alternative<MessageExtra const *>(v) )
        {
            obj_ = ObjectProxy(std::get<MessageExtra const *>(v));
        }
    }

    substituted_t get_value(){
        if( value_.size() > 0 )
        {
            return value_;
        }
        else{
            return obj_.value();
        }
    }

    void next(std::string const & key){
        obj_ = obj_.next(key);
    }

    void next(unsigned ix)
    {
        obj_ = obj_.next(ix);
    }

    void modify( std::string const & modifier_def )
    {
        auto substituted = obj_.value();
        auto value = std::get<std::span<std::byte const>>(substituted);

        Modifier modifier { modifier_def };

        value_ = modifier.modify(value);
    }
};


template<typename Rule>
struct action {};

template<>
struct action<grammar::identifier> {
    template<typename Input>
    static void apply(const Input & in, EvalState & state) {
        state.start(in.string());
    }
};

template<>
struct action<grammar::property> {
    template<typename Input>
    static void apply(const Input & in, EvalState & state) {
        auto p = in.string();
        state.next(std::string(p.begin() + 1, p.end()));
    }
};

template<>
struct action<grammar::modifier> {
    template<typename Input>
    static void apply(const Input & in, EvalState & state) {
        auto p = in.string();
        state.modify(std::string(p.begin() + 1, p.end()));
    }
};

template<>
struct action<grammar::index> {
    template<typename Input>
    static void apply(const Input & in, EvalState & state) {
        auto p = in.string();
        state.next(std::stoi(std::string(p.begin() + 1, p.end() - 1)));
    }
};


substituted_t SubsEngine::evaluate( string const & expression )
{
    EvalState state(env_);

    tao::pegtl::string_input in(expression, "");

    try{
        tao::pegtl::parse<grammar::expression, action>(in, state);

        return state.get_value();
    }
    catch(std::exception const & e)
    {
        throw;
    }
}
