cmake_minimum_required(VERSION 3.10)
project(Utils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(utils INTERFACE)

add_subdirectory(json)

target_include_directories(utils INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/json/include
)