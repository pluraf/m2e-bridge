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
    ${CMAKE_SOURCE_DIR}/external/lua-cjson
    "${CMAKE_CURRENT_BINARY_DIR}"
)

find_package(google_cloud_cpp_storage REQUIRED)
find_package(google_cloud_cpp_pubsub REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(civetweb CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZeroMQ REQUIRED libzmq)
find_package(CURL REQUIRED)
find_package(aws-cpp-sdk-core CONFIG REQUIRED)
find_package(aws-cpp-sdk-s3 CONFIG REQUIRED)
find_package(Poco REQUIRED Foundation)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

message(STATUS "Using protobuf ${Protobuf_VERSION}")
message(STATUS "Using gRPC ${gRPC_VERSION}")

option(protobuf_MODULE_COMPATIBLE TRUE)

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_GRPCPP gRPC::grpc++)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(api_proto "./protos/agent_api_modbus.proto" ABSOLUTE)
get_filename_component(api_proto_path "${api_proto}" PATH)

# Generated sources
set(api_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/agent_api_modbus.pb.cc")
set(api_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/agent_api_modbus.pb.h")
set(api_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/agent_api_modbus.grpc.pb.cc")
set(api_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/agent_api_modbus.grpc.pb.h")

add_custom_command(
      OUTPUT "${api_proto_srcs}" "${api_proto_hdrs}" "${api_grpc_srcs}" "${api_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${api_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${api_proto}"
      DEPENDS "${api_proto}")

find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/jwt.h"
            PATH_SUFFIXES include
            PATHS ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})


add_executable(m2e-bridge
    ${SOURCES}
    ${MQTT_SOURCES}
    ${api_grpc_srcs}
    ${api_grpc_hdrs}
    ${api_proto_srcs}
    ${api_proto_hdrs}
)

target_compile_definitions(m2e-bridge PRIVATE JWT_DISABLE_PICOJSON)
target_compile_definitions(m2e-bridge PRIVATE OPENSSL)

target_link_libraries(m2e-bridge PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(m2e-bridge PRIVATE google-cloud-cpp::pubsub)
target_link_libraries(m2e-bridge PRIVATE google-cloud-cpp::storage)
target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3as.a")
target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3cs.a")
target_link_libraries(m2e-bridge PRIVATE civetweb::civetweb-cpp)
target_link_libraries(m2e-bridge PRIVATE OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(m2e-bridge PRIVATE SQLite::SQLite3)
target_link_libraries(m2e-bridge PRIVATE fmt::fmt)
target_link_libraries(m2e-bridge PRIVATE ${ZeroMQ_LIBRARIES})
target_link_libraries(m2e-bridge PRIVATE CURL::libcurl)
target_link_libraries(m2e-bridge PRIVATE aws-cpp-sdk-core)
target_link_libraries(m2e-bridge PRIVATE aws-cpp-sdk-s3)
target_link_libraries(m2e-bridge PRIVATE cbor)
target_link_libraries(m2e-bridge PRIVATE Poco::Foundation)
target_link_libraries(m2e-bridge PRIVATE Poco::Foundation)
target_link_libraries(m2e-bridge PRIVATE
    absl::check
    absl::flags
    absl::flags_parse
    absl::log
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
)
target_link_libraries(m2e-bridge PRIVATE ${LUA_LIBRARIES})
target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/lua-cjson/build/cjson.a")
target_link_libraries(m2e-bridge PRIVATE libpqxx::pqxx)

target_include_directories(m2e-bridge PRIVATE ${JWT_CPP_INCLUDE_DIRS})

option(ENABLE_TESTS OFF)
message(STATUS "ENABLE_TESTS is set to ${ENABLE_TESTS}")

if(ENABLE_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)

    file(GLOB_RECURSE TESTS_HEASERS "tests/*.h")

    add_executable(tests
        ${SOURCES}
        ${MQTT_SOURCES}
        ${api_grpc_srcs}
        ${api_grpc_hdrs}
        ${api_proto_srcs}
        ${api_proto_hdrs}
    )

    target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_include_directories(tests PRIVATE ${JWT_CPP_INCLUDE_DIRS})
    target_include_directories(tests PRIVATE ${JWT_CPP_INCLUDE_DIRS})

    target_link_libraries(tests PRIVATE
        Catch2::Catch2WithMain
        nlohmann_json::nlohmann_json
        google-cloud-cpp::pubsub
        google-cloud-cpp::storage
        civetweb::civetweb-cpp
        OpenSSL::SSL
        OpenSSL::Crypto
        SQLite::SQLite3
        fmt::fmt
        CURL::libcurl
        aws-cpp-sdk-core
        aws-cpp-sdk-s3
        ${ZeroMQ_LIBRARIES}
        cbor
        Poco::Foundation
        ${LUA_LIBRARIES}
        "${CMAKE_SOURCE_DIR}/external/lua-cjson/build/cjson.a"
        "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3as.a"
        "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3cs.a"
    )
    enable_testing()
    add_test(NAME Tests COMMAND tests)
endif()

#target_link_libraries(MyProject PRIVATE fmt::fmt)

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
