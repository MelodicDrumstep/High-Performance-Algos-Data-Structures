cmake_minimum_required(VERSION 3.10)
project(BTree LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g -mavx2)
endif()

add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

add_executable(B_tree B_tree.cpp B_tree.hpp)

target_include_directories(B_tree PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../binary_search
)

target_link_libraries(B_tree PRIVATE utils)