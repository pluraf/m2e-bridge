#ifndef __M2E_BRIDGE_JWT_VALIDATION_H__
#define __M2E_BRIDGE_JWT_VALIDATION_H__

#include <string>

std::string load_public_key(const std::string& path);

bool jwt_verify(const std::string& jwt, const std::string& public_key);

#endif 
