#ifndef __M2E_BRIDGE_ALIASES_H__
#define __M2E_BRIDGE_ALIASES_H__


#include <vector>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>

#include <nlohmann/json.hpp>


namespace chrono = std::chrono;

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

using std::string;
using std::vector;
using std::map;
using std::set;
using std::stringstream;
using std::pair;
using std::time_t;


typedef pair<string,string> hops_t;


#endif  // __M2E_BRIDGE_ALIASES_H__
