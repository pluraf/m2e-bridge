#include <iostream>
#include <fstream>

#include "jwt_helpers.h"
#include <jwt-cpp/jwt.h>


std::string load_public_key(const std::string& path) {
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
