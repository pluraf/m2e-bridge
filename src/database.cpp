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
#include <string>
#include <vector>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdexcept>

#include "global_config.h"
#include "database.h"


ServiceType get_service_type(string const & val){
    if(val == "gcp") return ServiceType::GCP;
    if(val == "mqtt311") return ServiceType::MQTT311;
    if(val == "mqtt50") return ServiceType::MQTT50;
    if(val == "email") return ServiceType::EMAIL;
    if(val == "aws") return ServiceType::AWS;
    if(val == "http") return ServiceType::HTTP;
    if(val == "azure") return ServiceType::AZURE;
    return ServiceType::NONE;
}


std::string service_type_to_string(ServiceType ct){
     switch (ct) {
        case ServiceType::GCP: return "gcp";
        case ServiceType::MQTT311: return "mqtt311";
        case ServiceType::MQTT50: return "mqtt50";
        case ServiceType::EMAIL: return "email";
        case ServiceType::AWS: return "aws";
        case ServiceType::HTTP: return "http";
        case ServiceType::AZURE: return "azure";
        case ServiceType::NONE: return "";
        default: return "";
    }
}


AuthType get_auth_type(std::string val){
    if(val == "jwt_es256")
        return AuthType::JWT_ES256;
    else if(val == "password")
        return AuthType::PASSWORD;
    else if(val == "service_key")
        return AuthType::SERVICE_KEY;
    else if(val == "access_key")
        return AuthType::ACCESS_KEY;
    else if(val == "bearer")
        return AuthType::BEARER;
    else if(val == "basic")
        return AuthType::BASIC;
    else
        return AuthType::NONE;
}


std::string auth_type_to_string(AuthType at){
    switch (at) {
        case AuthType::JWT_ES256: return "jwt_es256";
        case AuthType::PASSWORD: return "password";
        case AuthType::SERVICE_KEY: return "service_key";
        case AuthType::ACCESS_KEY: return "access_key";
        case AuthType::BASIC: return "basic";
        case AuthType::BEARER: return "bearer";
        case AuthType::NONE: return "";
        default: return "";
    }
}


void print_authbundle(const AuthBundle& bundle) {
    std::cout << "AuthBundle ID: " << bundle.authbundle_id << std::endl;
    std::cout << "Service Type: " << service_type_to_string(bundle.service_type) << std::endl;
    std::cout << "Auth Type: " << auth_type_to_string(bundle.auth_type) << std::endl;
    std::cout << "Username: " << bundle.username << std::endl;
    std::cout << "Password: " << bundle.password << std::endl;
    std::cout << "Keyname: " << bundle.keyname << std::endl;
    std::cout << "Keydata: " << bundle.keydata << std::endl;
    std::cout << "Description: " << bundle.description << std::endl;
}
