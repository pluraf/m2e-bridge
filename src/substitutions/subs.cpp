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


#include <tao/pegtl.hpp>

#include "subs.hpp"


class ObjectProxy{
    std::variant<Message *, nlohmann::json const *, std::vector<std::string> const *, std::string const *, std::map<std::string, std::string> const *> obj_;
public:
    ObjectProxy() = default;

    ObjectProxy(Message * msg){
        obj_ = msg;
    }

    ObjectProxy(nlohmann::json const * j){
        obj_ = j;
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
                return ObjectProxy(& std::get<Message *>(obj_)->get_json());
            }else if(key == "TOPIC_LEVELS"){
                return ObjectProxy(& std::get<Message *>(obj_)->get_topic_levels());
            }else if(key == "TOPIC"){
                return ObjectProxy(& std::get<Message *>(obj_)->get_topic());
            }
        }else if(std::holds_alternative<nlohmann::json const *>(obj_)){
            return ObjectProxy(& std::get<nlohmann::json const *>(obj_)->at(key));
        }else if(std::holds_alternative<std::map<std::string, std::string> const *>(obj_)){
            return ObjectProxy(& std::get<std::map<std::string, std::string> const *>(obj_)->at(key));

        }
        throw std::out_of_range(key);
    }

    ObjectProxy next(unsigned ix){
        if(std::holds_alternative<std::vector<std::string> const *>(obj_)){
            return ObjectProxy(& std::get<std::vector<std::string> const *>(obj_)->at(ix));
        }else if(std::holds_alternative<nlohmann::json const *>(obj_)){
            return ObjectProxy(& std::get<nlohmann::json const *>(obj_)->at(ix));
        }
        throw std::invalid_argument("Index access is not available!");
    }

    std::string value(){
        if(std::holds_alternative<std::string const *>(obj_)){
            return * std::get<std::string const *>(obj_);
        }else if(std::holds_alternative<nlohmann::json const *>(obj_)){
            nlohmann::json const * j = std::get<nlohmann::json const *>(obj_);
            if(j->is_string()){
                return std::get<nlohmann::json const *>(obj_)->get<std::string>();
            }else if(j->is_number_integer()){
                return std::to_string(std::get<nlohmann::json const *>(obj_)->template get<unsigned>());
            }
        }
        throw std::runtime_error("Value is not a string!");
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

    void next(std::string const & key){
        obj_ = obj_.next(key);
    }

    void next(unsigned ix){
        obj_ = obj_.next(ix);
    }

    std::string value(){
        return obj_.value();
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


string SubsEngine::evaluate(string const & expression){
    EvalState state(env_);
    tao::pegtl::string_input in(expression, "");
    try{
        tao::pegtl::parse<grammar::expression, action>(in, state);
        return state.value();
    }catch(std::exception const & e){
        throw;
    }
}