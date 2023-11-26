cmake_minimum_required(VERSION 2.5)
project(netpoll)

set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++20")

aux_source_directory(. SRC_LIST)

add_library(netpoll SHARED ${SRC_LIST})
