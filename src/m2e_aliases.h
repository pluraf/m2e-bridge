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


#ifndef __M2E_BRIDGE_ALIASES_H__
#define __M2E_BRIDGE_ALIASES_H__


#include <vector>
#include <string>
#include <string_view>
#include <map>
#include <unordered_map>
#include <set>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cstddef>
#include <stdexcept>

#include <nlohmann/json.hpp>


namespace chrono = std::chrono;

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

using bytes = std::vector<std::byte>;
using uchars = std::vector<unsigned char>;

using std::string;
using std::string_view;
using std::vector;
using std::map;
using std::unordered_map;
using std::set;
using std::stringstream;
using std::pair;
using std::time_t;
using std::byte;


typedef pair<string,string> hops_t;


template<typename T>
T lexical_cast(std::string const &);


template<typename T>
std::string lexical_cast(T const &);


#endif  // __M2E_BRIDGE_ALIASES_H__
