cmake_minimum_required(VERSION 2.6)

project(play)
set(PLAY_DEPLIBS ${PLAY_DEPLIBS} asound rt m dl fftw3)
set(APLAY_PWD "\"${PROJECT_BINARY_DIR}\"")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread -std=c++14 " ${DEBUG})

if (NOT DEBUG)
    set(CMAKE_BUILD_TYPE Release)
else()
    set(CMAKE_BUILD_TYPE Debug)
endif()

configure_file("${PROJECT_SOURCE_DIR}/audio.h.in" "${PROJECT_SOURCE_DIR}/audio.h")
include_directories("${PROJECT_BINARY_DIR}/build")

add_executable(aplay aplay.c)
target_link_libraries(aplay ${PLAY_DEPLIBS})

add_library(play SHARED audio.cpp)
