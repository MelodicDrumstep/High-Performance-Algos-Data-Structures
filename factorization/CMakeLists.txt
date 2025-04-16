cmake_minimum_required(VERSION 3.10)
project(Factorization LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g)
endif()

add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

add_executable(factorization factorization.cpp factorization.hpp constexpr_bitmap.hpp)

target_include_directories(factorization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(factorization PRIVATE utils)