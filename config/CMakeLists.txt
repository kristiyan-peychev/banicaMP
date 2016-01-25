cmake_minimum_required(VERSION 2.6)

project(config)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)
set(CMAKE_BUILD_TYPE Release)
add_library(config config.cpp)

