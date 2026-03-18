cmake_minimum_required(VERSION 4.0)

project(m2e-brdige)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


option(WITH_GCP_CLOUD_STORAGE_CONNECTOR "" OFF)
option(WITH_GCP_PUBSUB_CONNECTOR "" OFF)
option(WITH_AWS_S3_CONNECTOR "" OFF)
option(WITH_SQLITE_CONNECTOR "" OFF)
option(WITH_POSTGRESQL_CONNECTOR "" OFF)
option(WITH_MODBUS_CONNECTOR "" OFF)
option(WITH_MQTT_CONNECTOR "" OFF)
option(WITH_EMAIL_CONNECTOR "" OFF)
option(WITH_HTTP_CONNECTOR "" OFF)
option(WITH_SLACK_CONNECTOR "" OFF)
option(WITH_AZURE_SERVICE_BUS_CONNECTOR "" OFF)

option(WITH_HTTP_LISTENER "" OFF)

option(WITH_LUA "" OFF)

option(WITH_ZEROMQ_API "" OFF)
option(WITH_HTTP_API "" OFF)

option(ENABLE_TESTS OFF)

set(_WITH_CIVETWEB OFF)
set(_WITH_CURL OFF)
option(_WITH_OPENSSL "" OFF)
option(_WITH_OPENCV "" OFF)


if( WITH_HTTP_API OR WITH_HTTP_LISTENER )
    set(_WITH_CIVETWEB ON)
endif()

if( WITH_EMAIL_CONNECTOR OR WITH_HTTP_CONNECTOR )
    set(_WITH_CURL ON)
endif()


set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
#add_definitions(-DPROJECT_SOURCE_DIR=\"${SOURCE_DIR}\")

include_directories(
    ${SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/include
    ${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/src
    ${CMAKE_SOURCE_DIR}/external/lua-cjson
    "${CMAKE_CURRENT_BINARY_DIR}"
)

if( _WITH_CIVETWEB )
    find_package(civetweb CONFIG REQUIRED)
endif()

if( _WITH_CURL )
    find_package(CURL REQUIRED)
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(Poco REQUIRED Foundation)
find_package(fmt REQUIRED)
find_package(PkgConfig REQUIRED)

if( WITH_GCP_PUBSUB_CONNECTOR )
    find_package(google_cloud_cpp_pubsub REQUIRED)

endif()

if( WITH_GCP_CLOUD_STORAGE_CONNECTOR )
    find_package(google_cloud_cpp_storage REQUIRED)
endif()

if( WITH_AWS_S3_CONNECTOR )
    find_package(aws-cpp-sdk-core CONFIG REQUIRED)
    find_package(aws-cpp-sdk-s3 CONFIG REQUIRED)
endif()

if( WITH_LUA )
    find_package(Lua REQUIRED)
endif()

if( WITH_SQLITE_CONNECTOR )
    find_package(SQLite3 REQUIRED)
endif()

if( WITH_POSTGRESQL_CONNECTOR )
    find_package(libpqxx CONFIG REQUIRED)
endif()

if( _WITH_OPENSSL_LIBRARY )
    find_package(OpenSSL REQUIRED)
endif()

if( WITH_IMAGE_HANDLERS )
    find_package(OpenCV CONFIG REQUIRED)
endif()

if( WITH_MODBUS_CONNECTOR )
    find_package(Protobuf CONFIG REQUIRED)
    find_package(gRPC CONFIG REQUIRED)

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
endif()


if( WITH_ZEROMQ_API )
    pkg_check_modules(ZeroMQ REQUIRED libzmq)
endif()


##############################################################################

set(CORE_SOURCES
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/pipeline.cpp
    ${SOURCE_DIR}/pipeline_supervisor.cpp
    ${SOURCE_DIR}/shared_objects.cpp
    ${SOURCE_DIR}/internal_queue.cpp
    ${SOURCE_DIR}/validator.cpp
    ${SOURCE_DIR}/substitutions/subs.cpp
    ${SOURCE_DIR}/database/authbundle.cpp
)

if( WITH_MQTT_CONNECTOR )
    file(GLOB MQTT_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/src/*.cpp")
    list(APPEND OPTIONAL_SOURCES ${MQTT_SOURCES})
endif()

if( WITH_MODBUS_CONNECTOR )
    list(APPEND OPTIONAL_SOURCES ${SOURCE_DIR}/connectors/modbus/modbus_connector.cpp)
    list(APPEND OPTIONAL_SOURCES
        ${api_grpc_srcs}
        ${api_grpc_hdrs}
        ${api_proto_srcs}
        ${api_proto_hdrs}
    )
endif()

if( WITH_HTTP_LISTENER )
    list(APPEND OPTIONAL_SOURCES ${SOURCE_DIR}/listeners/http_listener.cpp)
endif()

if( WITH_ZEROMQ_API )
    list(APPEND OPTIONAL_SOURCES ${SOURCE_DIR}/zmq_api.cpp)
    list(APPEND OPTIONAL_SOURCES ${SOURCE_DIR}/zmq_listner.cpp)
endif()

if( WITH_HTTP_API )
    list(APPEND OPTIONAL_SOURCES
        ${SOURCE_DIR}/rest_api.cpp
        ${SOURCE_DIR}/api_helpers.cpp
        ${SOURCE_DIR}/jwt_helpers.cpp
    )
endif()


add_executable(m2e-bridge
    ${CORE_SOURCES}
    ${OPTIONAL_SOURCES}
)

##############################################################################

target_compile_definitions(m2e-bridge PRIVATE
    $<$<BOOL:${WITH_SQLITE_CONNECTOR}>:WITH_SQLITE_CONNECTOR>
    $<$<BOOL:${WITH_POSTGRESQL_CONNECTOR}>:WITH_POSTGRESQL_CONNECTOR>
    $<$<BOOL:${WITH_MODBUS_CONNECTOR}>:WITH_MODBUS_CONNECTOR>
    $<$<BOOL:${WITH_SLACK_CONNECTOR}>:WITH_SLACK_CONNECTOR>
    $<$<BOOL:${WITH_MQTT_CONNECTOR}>:WITH_MQTT_CONNECTOR>
    $<$<BOOL:${WITH_GCP_PUBSUB_CONNECTOR}>:WITH_GCP_PUBSUB_CONNECTOR>
    $<$<BOOL:${WITH_GCP_CLOUD_STORAGE_CONNECTOR}>:WITH_GCP_CLOUD_STORAGE_CONNECTOR>
    $<$<BOOL:${WITH_AZURE_SERVICE_BUS_CONNECTOR}>:WITH_AZURE_SERVICE_BUS_CONNECTOR>
    $<$<BOOL:${WITH_AWS_S3_CONNECTOR}>:WITH_AWS_S3_CONNECTOR>
    $<$<BOOL:${WITH_HTTP_CONNECTOR}>:WITH_HTTP_CONNECTOR>
    $<$<BOOL:${WITH_EMAIL_CONNECTOR}>:WITH_EMAIL_CONNECTOR>

    $<$<BOOL:${WITH_LUA}>:WITH_LUA>

    $<$<BOOL:${WITH_IMAGE_HANDLERS}>:WITH_IMAGE_HANDLERS>
)


find_path(JWT_CPP_INCLUDE_DIRS "jwt-cpp/jwt.h"
            PATH_SUFFIXES include
            PATHS ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})


target_compile_definitions(m2e-bridge PRIVATE JWT_DISABLE_PICOJSON)
target_link_libraries(m2e-bridge PRIVATE nlohmann_json::nlohmann_json)

##############################################################################

if (_WITH_CIVETWEB )
    target_link_libraries(m2e-bridge PRIVATE civetweb::civetweb-cpp)
endif()

if( _WITH_CURL )
    target_link_libraries(m2e-bridge PRIVATE CURL::libcurl)
endif()

if( WITH_GCP_PUBSUB_CONNECTOR )
    target_link_libraries(m2e-bridge PRIVATE google-cloud-cpp::pubsub)
endif()

if( WITH_GCP_CLOUD_STORAGE_CONNECTOR )
    target_link_libraries(m2e-bridge PRIVATE google-cloud-cpp::storage)
endif()

if( WITH_SQLITE_CONNECTOR )
    target_link_libraries(m2e-bridge PRIVATE SQLite3::SQLite3)
endif()

if( WITH_AWS_S3_CONNECTOR )
    target_link_libraries(m2e-bridge PRIVATE aws-cpp-sdk-core)
    target_link_libraries(m2e-bridge PRIVATE aws-cpp-sdk-s3)
endif()

if( WITH_MODBUS_CONNECTOR )
    target_link_libraries(m2e-bridge PRIVATE
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF}
    )
endif()

if( WITH_POSTGRESQL_CONNECTOR )
    target_link_libraries(m2e-bridge PRIVATE libpqxx::pqxx)
endif()

if( WITH_MQTT_CONNECTOR )
    target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3as.a")
    target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3cs.a")
endif()

if( _WITH_OPENSSL_LIBRARY )
    target_link_libraries(m2e-bridge PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if( WITH_IMAGE_HANDLERS )
    target_link_libraries(m2e-bridge PRIVATE opencv_core opencv_imgproc opencv_imgcodecs)
endif()

if( WITH_ZEROMQ_API )
    target_link_libraries(m2e-bridge PRIVATE ${ZeroMQ_LIBRARIES})
endif()

if( WITH_LUA )
    target_link_libraries(m2e-bridge PRIVATE ${LUA_LIBRARIES})
    target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/lua-cjson/build/cjson.a")
endif()

target_include_directories(m2e-bridge PRIVATE ${JWT_CPP_INCLUDE_DIRS})
target_link_libraries(m2e-bridge PRIVATE fmt::fmt)
target_link_libraries(m2e-bridge PRIVATE Poco::Foundation)
target_link_libraries(m2e-bridge PRIVATE Poco::Foundation)
target_link_libraries(m2e-bridge PRIVATE cbor)

#target_link_libraries(m2e-bridge PRIVATE
#    absl::check
#    absl::flags
#    absl::flags_parse
#    absl::log
#)

if( ENABLE_TESTS )
    message(STATUS "ENABLE_TESTS is set to ${ENABLE_TESTS}")
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)

    file(GLOB_RECURSE TESTS_HEASERS "tests/*.h")

    add_executable(tests
        ${CORE_SOURCES}
        ${OPTIONAL_SOURCES}
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
