cmake_minimum_required(VERSION 2.5)
project(netpoll)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++20")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++20 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")


aux_source_directory(netpoll SRC_LIST)

add_library(netpoll SHARED ${SRC_LIST})

set(LIB_LIB
    netpoll
    pthread
)


add_executable(echoServer examples/echoServer.cc)
add_dependencies(echoServer netpoll)
target_link_libraries(echoServer ${LIB_LIB})
