cmake_minimum_required(VERSION 2.6)

project(ipc)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} " -std=c++11")

if (DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_BUILD_TYPE Debug)

include_directories("${PROJECT_BINARY_DIR}/build")


add_library(ipc SHARED interprocess.cpp)
