cmake_minimum_required(VERSION 3.9)

project(MCIS)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


set(CMAKE_BUILD_TYPE Debug)

include_directories("${PROJECT_SOURCE_DIR}/include")


add_library(MCIS_crc STATIC             ${PROJECT_SOURCE_DIR}/crc.c)
add_library(MCIS_discreteMath STATIC    ${PROJECT_SOURCE_DIR}/discreteMath.cpp) 
add_library(MCIS_config STATIC          ${PROJECT_SOURCE_DIR}/MCIS_config.cpp) 
add_library(MCIS_fileio STATIC          ${PROJECT_SOURCE_DIR}/MCIS_fileio.cpp) 
add_library(MCIS_MDA STATIC             ${PROJECT_SOURCE_DIR}/MCIS_MDA.cpp)
add_library(MCIS_xplane_sock STATIC     ${PROJECT_SOURCE_DIR}/MCIS_xplane_sock.cpp)
add_library(MCIS_MB_interface STATIC    ${PROJECT_SOURCE_DIR}/MCIS_MB_interface.cpp)
target_link_libraries(MCIS_MB_interface MCIS_xplane_sock)





add_executable(MCIS ${PROJECT_SOURCE_DIR}/MCIS.cpp)
target_link_libraries(MCIS MCIS_crc MCIS_discreteMath MCIS_config)
target_link_libraries(MCIS MCIS_fileio MCIS_MDA) 
target_link_libraries(MCIS MCIS_xplane_sock)
target_link_libraries(MCIS MCIS_MB_interface)
target_link_libraries(MCIS -lpthread -lncurses)
target_compile_features(MCIS PUBLIC cxx_std_11)

add_executable(MCIS-offline ${PROJECT_SOURCE_DIR}/MCIS-offline.cpp)
target_link_libraries(MCIS-offline MCIS_crc MCIS_discreteMath MCIS_config)
target_link_libraries(MCIS-offline MCIS_fileio MCIS_MDA) 
target_compile_features(MCIS-offline PUBLIC cxx_std_11)
