cmake_minimum_required(VERSION 2.6)

project(interprocess)
set(INCLUDE_DIR "\"${PROJECT_BINARY_DIR}/interprocess.h\"")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread -std=c++14")

if (DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_BUILD_TYPE Debug)

configure_file("${PROJECT_SOURCE_DIR}/../get.h.in" "${PROJECT_SOURCE_DIR}/../get.h")
include_directories("${PROJECT_BINARY_DIR}/build")


add_library(interprocess SHARED interprocess.cpp)
