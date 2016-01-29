cmake_minimum_required(VERSION 2.6)

project(audio_linux)
set(APLAY_DEPLIBS ${APLAY_DEPLIBS} asound rt m dl fftw3)
set(APLAY_PWD "\"${PROJECT_BINARY_DIR}\"")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread -std=c++14")

if (DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

configure_file("${PROJECT_SOURCE_DIR}/audio.h.in" "${PROJECT_SOURCE_DIR}/audio.h")
include_directories("${PROJECT_BINARY_DIR}/build")

add_executable(aplay aplay.c)
target_link_libraries(aplay ${APLAY_DEPLIBS})

add_library(audio audio.cpp aplay.cpp)
target_link_libraries(audio ${APLAY_DEPLIBS})
