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


#include "rest_api_helpers.h"


std::vector<std::string> get_last_segments(char const * uri, int count){
    vector<string> segments;
    string segment;
    std::stringstream ss(uri);
    while(std::getline(ss, segment, '/')){
        segments.push_back(segment);
    }
    if(segments.size() < count){
        throw std::out_of_range("");
    }
    return vector<string>(segments.end() - count, segments.end());
}

int parse_request_body(struct mg_connection * conn, json & pipeline_data){
    char buf[1024];
    int length = mg_read(conn, buf, sizeof(buf));
    buf[length] = '\0';
    try {
        pipeline_data = ordered_json::parse(buf);
        return 0;
    }
    catch(json::parse_error &e){
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

int parse_pipeline_ids(struct mg_connection *conn, std::vector<std::string> &pipeline_ids){
    char buf[1024];
    int length = mg_read(conn, buf, sizeof(buf));
    buf[length] = '\0';
    try {
        json requestData = json::parse(buf);

        if(! requestData.is_array()) return 1;

        for(const auto &item : requestData) {
            pipeline_ids.push_back(item.get<std::string>());
        }

        return 0;
    }
    catch (json::parse_error &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
