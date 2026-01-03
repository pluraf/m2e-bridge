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


#ifndef __M2E_BRIDGE_MODIFIER_H__
#define __M2E_BRIDGE_MODIFIER_H__


#include <memory>

#include "m2e_aliases.h"

#include "resize.h"


struct ModifierType
{
    enum class Type{ UNKN, RESIZE };

    static std::string to_string(ModifierType::Type v)
    {
        switch(v){
            case Type::UNKN: return "unkn";
            case Type::RESIZE: return "resize";
            default: throw std::invalid_argument("Invalid ImageOperation");
        }
    }

    static ModifierType::Type from_string(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c) { return std::tolower(c); });

        if( str == "unkn" ){ return ModifierType::Type::UNKN; }
        if( str == "resize" ){ return ModifierType::Type::RESIZE; }

        throw std::invalid_argument("Unknown modifier: " + str);
    }
};


class Modifier
{
    std::unique_ptr<ModifierInternal> modifier_ptr_;

public:
    Modifier( std::string notation )
    {
        auto [ mtype, mparams ] = parse_notation(notation);

        switch( mtype )
        {
        case ModifierType::Type::RESIZE:
            modifier_ptr_ = std::make_unique<ResizeModifier>(mparams);
            break;
        }
    }

    virtual ~Modifier() {}

    uchars modify( std::span<std::byte> data )
    {
        return modifier_ptr_->modify( data );
    }

    static json get_schema(){
        return json {};
    }

private:
    std::pair<ModifierType::Type, string> parse_notation( string const & notation )
    {
        ModifierType::Type mtype;
        string mparams;

        auto params_start = notation.find(",");
        if( params_start != string::npos )
        {
            mtype = ModifierType::from_string(notation.substr( 0, params_start));
            mparams = notation.substr( params_start + 1 );
        }else{
            mtype = ModifierType::from_string(notation);
        }

        return { mtype, mparams };
    }
};


#endif  // __M2E_BRIDGE_MODIFIER_H__
