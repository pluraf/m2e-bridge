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

#include "jwt_helpers.h"
#include <jwt-cpp/jwt.h>


std::string load_public_key(std::string const & path){
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open public key file.");
    }
    std::string key((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return key;
}

bool jwt_verify(const std::string& jwt_token, const std::string& public_key) {
    try {
        auto decoded_token = jwt::decode(jwt_token);

        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::es256(public_key, "", "", ""));

        verifier.verify(decoded_token);

        return true;
    }
    catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            ERR_print_errors_fp(stderr);
    }

    return false;
}
