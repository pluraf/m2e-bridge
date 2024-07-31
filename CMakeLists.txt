cmake_minimum_required(VERSION 3.30)

project(m2e-brdige)

set(CMAKE_CXX_STANDARD 20)

find_package(google_cloud_cpp_pubsub REQUIRED)

add_executable(MyProject main.cpp)

target_link_libraries(MyProject PRIVATE fmt::fmt)



#set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
