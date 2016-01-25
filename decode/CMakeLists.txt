cmake_minimum_required(VERSION 2.6)

project(decode)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 ")
if (DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()
add_library(decode FLAC_decoder.cpp MPEG_decoder.cpp get.cpp)
