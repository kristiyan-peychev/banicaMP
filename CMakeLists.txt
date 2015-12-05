cmake_minimum_required(VERSION 2.8)

project(banicaMP)

option(debug "DEBUG" "")

add_subdirectory(config config/build)
add_subdirectory(decode decode/build)
add_subdirectory(memory memory/build)

add_subdirectory(playback playback)
