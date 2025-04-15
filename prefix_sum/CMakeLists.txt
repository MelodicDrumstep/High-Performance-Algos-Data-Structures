cmake_minimum_required(VERSION 3.10)
project(PrefixSum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g -mavx2)
endif()

add_executable(prefix_sum prefix_sum.cpp prefix_sum.hpp aligned_allocator.hpp)

target_include_directories(prefix_sum PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})