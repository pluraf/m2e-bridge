cmake_minimum_required(VERSION 3.30)

project(m2e-brdige)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")

include_directories(${SOURCE_DIR})

find_package(google_cloud_cpp_pubsub REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(m2e-bridge ${SOURCES})

target_link_libraries(m2e-bridge PRIVATE nlohmann_json::nlohmann_json)


#target_link_libraries(MyProject PRIVATE fmt::fmt)

#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
