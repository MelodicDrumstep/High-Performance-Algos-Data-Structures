cmake_minimum_required(VERSION 3.10)
project(StableVector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -pedantic -O3 -g)
endif()

add_subdirectory(../utils ${CMAKE_BINARY_DIR}/utils)

# Test executable
add_executable(stable_vector_test stable_vector_test.cpp)

target_include_directories(stable_vector_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(stable_vector_test PRIVATE utils)

# WSS test executable
add_executable(wss_test wss_test.cpp)

target_include_directories(wss_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(wss_test PRIVATE utils)