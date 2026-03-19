
message(STATUS "ENABLE_TESTS is set to ${ENABLE_TESTS}")

enable_testing()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.13.0
)
FetchContent_MakeAvailable(Catch2)

file(GLOB_RECURSE TEST_SOURCES *.cpp)

include(../cmake/target.cmake)
include(Catch)

add_target(m2e-bridge-tests)
target_sources(m2e-bridge-tests PRIVATE ${TEST_SOURCES})

target_link_libraries(m2e-bridge-tests PRIVATE
    Catch2::Catch2WithMain
)

target_compile_definitions(m2e-bridge-tests PRIVATE
    TEST_CONFIG_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/configs\"
)

catch_discover_tests(m2e-bridge-tests)

message(${CMAKE_CURRENT_SOURCE_DIR})
