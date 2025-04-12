cmake_minimum_required(VERSION 3.10)
project(Division LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g)
endif()

add_executable(division division.cpp division.hpp)

target_include_directories(division PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})