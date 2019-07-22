cmake_minimum_required(VERSION 3.9)

project(MCIS)

set(CMAKE_VERBOSE_MAKEFILE OFF)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


set(CMAKE_BUILD_TYPE Debug)

include_directories("${PROJECT_SOURCE_DIR}/include")

add_library(MCIS_util STATIC            ${PROJECT_SOURCE_DIR}/MCIS_util.cpp)
add_library(MCIS_crc STATIC             ${PROJECT_SOURCE_DIR}/crc.c)
add_library(MCIS_discreteMath STATIC    ${PROJECT_SOURCE_DIR}/discreteMath.cpp) 
add_library(MCIS_config STATIC          ${PROJECT_SOURCE_DIR}/MCIS_config.cpp)
add_library(MCIS_fileio STATIC          ${PROJECT_SOURCE_DIR}/MCIS_fileio.cpp) 
add_library(MCIS_MDA STATIC             ${PROJECT_SOURCE_DIR}/MCIS_MDA.cpp)
add_library(MCIS_xplane_sock STATIC     ${PROJECT_SOURCE_DIR}/MCIS_xplane_sock.cpp)
add_library(MCIS_MB_interface STATIC    ${PROJECT_SOURCE_DIR}/MCIS_MB_interface.cpp)

target_link_libraries(MCIS_discreteMath MCIS_config)
target_link_libraries(MCIS_config MCIS_crc MCIS_util)
target_link_libraries(MCIS_fileio MCIS_discreteMath)
target_link_libraries(MCIS_MDA MCIS_discreteMath MCIS_config)
target_link_libraries(MCIS_xplane_sock MCIS_discreteMath MCIS_util -pthread)

target_link_libraries(MCIS_MB_interface MCIS_xplane_sock MCIS_MDA MCIS_fileio)
target_link_libraries(MCIS_MB_interface MCIS_discreteMath MCIS_util -pthread)






add_executable(MCIS ${PROJECT_SOURCE_DIR}/MCIS.cpp)
target_link_libraries(MCIS MCIS_config MCIS_MB_interface)
target_link_libraries(MCIS -lncurses -lconfig++)
target_compile_features(MCIS PUBLIC cxx_std_11)
target_compile_options(MCIS PUBLIC -Wall -Wextra -pedantic)

add_executable(MCIS-offline ${PROJECT_SOURCE_DIR}/MCIS-offline.cpp)
target_link_libraries(MCIS-offline MCIS_discreteMath MCIS_config MCIS_MDA MCIS_fileio)
target_compile_features(MCIS-offline PUBLIC cxx_std_11)
target_compile_options(MCIS-offline PUBLIC -Wall -Wextra -pedantic)
