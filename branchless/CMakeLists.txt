cmake_minimum_required(VERSION 3.10)
project(SumExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(sum sum.cpp sum.hpp)

target_include_directories(sum PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})