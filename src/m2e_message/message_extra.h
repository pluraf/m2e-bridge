/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2026 Pluraf Embedded AB <code@pluraf.com>

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


#ifndef __M2E_BRIDGE_MESSAGE_EXTRA_H__
#define __M2E_BRIDGE_MESSAGE_EXTRA_H__


#include <variant>

#include "m2e_aliases.h"
#include "message.h"


using bytes = std::vector<std::byte>;
using uchars = std::vector<unsigned char>;


class MessageExtra
{
    mutable string key_;
    map<string, std::variant<bytes, uchars, string>> extras_;

public:
    template< typename T >
    void add_extra( string const & key, T && data )
    {
        // Since we return the pointer to an extra in get_extra, we must be careful
        // with adding extras more than once...
        if( ! extras_.contains(key) ){
            extras_[key] = data;
        }
        else{
            throw std::logic_error("Extra already exist!");
        }
    }

    void set_key( string const & key ) const
    {
        key_ = key;
    }

    byte const * get_extra() const
    {
        auto & extra = extras_.at(key_);

        if( std::holds_alternative<bytes>(extra) )
        {
            return reinterpret_cast<byte const *>( & std::get<bytes>(extra).front() );
        }
        else if( std::holds_alternative<uchars>(extra) )
        {
            return reinterpret_cast<byte const *>( & std::get<uchars>(extra).front() );
        }
        else if( std::holds_alternative<string>(extra) )
        {
            return reinterpret_cast<byte const *>( & std::get<string>(extra).front() );
        }

        throw std::runtime_error("Unknown Extra type!");
    }

    size_t get_extra_size() const
    {
        auto & extra = extras_.at(key_);

        if( std::holds_alternative<bytes>(extra) )
        {
            return std::get<bytes>(extra).size();
        }
        else if( std::holds_alternative<uchars>(extra) )
        {
            return std::get<uchars>(extra).size();
        }
        else if( std::holds_alternative<string>(extra) )
        {
            return std::get<string>(extra).size();
        }

        throw std::runtime_error("Unknown Extra type!");
    }
};


#endif  // __M2E_BRIDGE_MESSAGE_EXTRA_H__