cmake_minimum_required(VERSION 2.6)

project(interprocess)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread -std=c++14")

if (DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_BUILD_TYPE Debug)

include_directories("${PROJECT_BINARY_DIR}/build")


add_library(interprocess SHARED interprocess.cpp)
