cmake_minimum_required(VERSION 3.25)

project(m2e-brdige)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
add_definitions(-DPROJECT_SOURCE_DIR=\"${SOURCE_DIR}\")

file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")
file(GLOB MQTT_SOURCES "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/src/*.cpp")

include_directories(
    ${SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/include
    ${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/src
)

find_package(google_cloud_cpp_pubsub REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(civetweb CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(fmt REQUIRED)
find_package(CURL REQUIRED)

find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/jwt.h"
            PATH_SUFFIXES include
            PATHS ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})


add_executable(m2e-bridge ${SOURCES} ${MQTT_SOURCES})

target_link_libraries(m2e-bridge PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(m2e-bridge PRIVATE google-cloud-cpp::pubsub)
target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3as.a")
target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3cs.a")
target_link_libraries(m2e-bridge PRIVATE civetweb::civetweb-cpp)
target_link_libraries(m2e-bridge PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(m2e-bridge PRIVATE SQLite::SQLite3)
target_link_libraries(m2e-bridge PRIVATE fmt::fmt)
target_link_libraries(m2e-bridge PRIVATE CURL::libcurl)

target_include_directories(m2e-bridge PRIVATE ${JWT_CPP_INCLUDE_DIRS})

#target_link_libraries(MyProject PRIVATE fmt::fmt)

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
