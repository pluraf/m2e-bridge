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


#include <tao/pegtl.hpp>
#include <cbor.h>

#include "subs.hpp"


class ObjectProxy{
    std::variant<
        Message *,
        nlohmann::json const *,
        std::vector<std::string> const *,
        std::string const *,
        std::map<std::string, std::string> const *,
        cbor_item_t const *
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
                    case CBOR_INT_8: return std::to_string(cbor_get_uint8(item));
                    case CBOR_INT_16: return std::to_string(cbor_get_uint16(item));
                    case CBOR_INT_32: return std::to_string(cbor_get_uint32(item));
                    case CBOR_INT_64: return std::to_string(cbor_get_uint64(item));
                }
            }
            else if(cbor_isa_bytestring(item)){
                return std::span<byte>(
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

        throw std::runtime_error("Value is not substitutable!");
    }
};


namespace grammar {
    using namespace tao::pegtl;

    struct dot : tao::pegtl::string<'.'> {};
    struct number : seq<> {};
    struct lbracket : tao::pegtl::string<'['> {};
    struct rbracket : tao::pegtl::string<']'> {};
    struct property : seq<dot, identifier_first, star<identifier_other>> {};
    struct index : seq<lbracket, plus<digit>, rbracket> {};
    struct expression : seq<identifier, plus<sor<property, index>>> {};
}


class EvalState {
    ObjectProxy obj_;
    EnvObjects & env_;
public:
    EvalState(EnvObjects & env): env_(env){
    }

    void start(std::string const & name){
        auto v = env_.at(name);
        if(std::holds_alternative<Message *>(v)){
            obj_ = ObjectProxy(std::get<Message *>(v));
        }else if(std::holds_alternative<json const *>(v)){
            obj_ = ObjectProxy(std::get<json const *>(v));
        }else if(std::holds_alternative<StringMap const *>(v)){
            obj_ = ObjectProxy(std::get<StringMap const *>(v));
        }
    }

    ObjectProxy & get_object(){
        return obj_;
    }

    void next(std::string const & key){
        obj_ = obj_.next(key);
    }

    void next(unsigned ix){
        obj_ = obj_.next(ix);
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
struct action<grammar::index> {
    template<typename Input>
    static void apply(const Input & in, EvalState & state) {
        auto p = in.string();
        state.next(std::stoi(std::string(p.begin() + 1, p.end() - 1)));
    }
};


substituted_t SubsEngine::evaluate(string const & expression){
    EvalState state(env_);
    tao::pegtl::string_input in(expression, "");
    try{
        tao::pegtl::parse<grammar::expression, action>(in, state);
        return state.get_object().value();
    }catch(std::exception const & e){
        throw;
    }
}
