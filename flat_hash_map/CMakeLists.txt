cmake_minimum_required(VERSION 3.10)
project(FlatHashMap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g)
endif()

# Add Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

# Main executable
add_executable(flat_hash_map flat_hash_map.cpp flat_hash_map.hpp)

target_include_directories(flat_hash_map PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(flat_hash_map PRIVATE utils)

# Test executable
add_executable(flat_hash_map_test flat_hash_map_test.cpp)

target_include_directories(flat_hash_map_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(flat_hash_map_test PRIVATE 
    GTest::gtest_main
    utils
)

# Enable testing
enable_testing()
add_test(NAME flat_hash_map_test COMMAND flat_hash_map_test)