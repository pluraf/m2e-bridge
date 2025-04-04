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


#include <iostream>
#include <fstream>


#include "api_helpers.h"


vector<string> get_last_segments(string_view uri, size_t count){
    vector<string> segments;
    size_t start = 0;
    size_t end = 0;
    while (end != std::string_view::npos) {
        end = uri.find('/', start);
        if(end == string_view::npos){
            if (start < uri.size()) {
                segments.push_back(string(uri.substr(start)));
            }
        }else{
            segments.push_back(string(uri.substr(start, end - start)));
            start = end + 1;
        }
    }
    return segments;
}


vector<string> get_last_segments(char const * uri, size_t count){
    vector<string> segments;
    string segment;
    std::stringstream ss(uri);
    while(std::getline(ss, segment, '/')){
        segments.push_back(segment);
    }
    if(count != 0 && segments.size() < count){
        throw std::out_of_range("");
    }
    if(count == 0) return segments;
    return vector<string>(segments.end() - count, segments.end());
}
