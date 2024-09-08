cmake_minimum_required(VERSION 3.30)

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

add_executable(m2e-bridge ${SOURCES} ${MQTT_SOURCES})

target_link_libraries(m2e-bridge PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(m2e-bridge PRIVATE google-cloud-cpp::pubsub)
target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3as.a")
target_link_libraries(m2e-bridge PRIVATE "${CMAKE_SOURCE_DIR}/external/paho.mqtt.cpp/externals/paho-mqtt-c/build/src/libpaho-mqtt3cs.a")

#target_link_libraries(MyProject PRIVATE fmt::fmt)

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
