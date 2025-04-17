cmake_minimum_required(VERSION 3.10)
project(Argmin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g -mavx2)
endif()

add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

add_executable(argmin argmin.cpp argmin.hpp)

target_include_directories(argmin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(argmin PRIVATE utils)