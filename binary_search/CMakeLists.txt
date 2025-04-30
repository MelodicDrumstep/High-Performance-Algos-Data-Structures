cmake_minimum_required(VERSION 3.10)
project(BinarySearch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g -mavx2 -mfma)
endif()

# Add utils if you use it (adjust the path as needed)
add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

# Common source files
set(BINARY_SEARCH_SOURCES
    binary_search.hpp
)

# Main binary_search executable
add_executable(binary_search binary_search.cpp ${BINARY_SEARCH_SOURCES})
target_include_directories(binary_search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(binary_search PRIVATE utils)

# Profile executable
add_executable(binary_search_profile binary_search_profile.cpp ${BINARY_SEARCH_SOURCES})
target_include_directories(binary_search_profile PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(binary_search_profile PRIVATE utils)