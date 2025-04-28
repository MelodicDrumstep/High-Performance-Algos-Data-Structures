cmake_minimum_required(VERSION 3.10)
project(BinarySearch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g)
endif()

add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

add_executable(binary_search binary_search.cpp binary_search.hpp)

target_include_directories(binary_search PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(binary_search PRIVATE utils)