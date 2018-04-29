cmake_minimum_required(VERSION 3.9)

project(MCIS)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(mbtest ${PROJECT_SOURCE_DIR}/mbtest.cpp)