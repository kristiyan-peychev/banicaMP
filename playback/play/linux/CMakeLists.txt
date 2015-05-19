cmake_minimum_required(VERSION 2.6)

project(play)
set(PLAY_DEPLIBS ${PLAY_DEPLIBS} asound rt m dl)
set(APLAY_PWD "\"${PROJECT_BINARY_DIR}\"")

configure_file("${PROJECT_SOURCE_DIR}/play.h.in" "${PROJECT_SOURCE_DIR}/play.h")
include_directories("${PROJECT_BINARY_DIR}/build")

add_executable(aplay aplay.c)
target_link_libraries(aplay ${PLAY_DEPLIBS})

add_library(play SHARED play.cpp)
