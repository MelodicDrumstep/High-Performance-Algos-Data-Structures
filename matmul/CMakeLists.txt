cmake_minimum_required(VERSION 3.10)
project(Matmul LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g -mavx2 -mfma)
endif()

add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

# Common source files
set(MATMUL_SOURCES matmul.hpp)

# Main matmul executable
add_executable(matmul matmul.cpp ${MATMUL_SOURCES})
target_include_directories(matmul PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(matmul PRIVATE utils)

# Profile executable
add_executable(matmul_profile matmul_profile.cpp ${MATMUL_SOURCES})
target_include_directories(matmul_profile PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(matmul_profile PRIVATE utils)