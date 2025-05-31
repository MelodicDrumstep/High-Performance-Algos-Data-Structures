cmake_minimum_required(VERSION 3.10)
project(FlatHashMap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g)
endif()

add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

add_executable(flat_hash_map flat_hash_map.cpp flat_hash_map.hpp)

target_include_directories(flat_hash_map PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(flat_hash_map PRIVATE utils)