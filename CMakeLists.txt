cmake_minimum_required(VERSION 2.8)

add_subdirectory(config config/build)
add_subdirectory(decode decode/build)
add_subdirectory(memory memory/build)

add_subdirectory(playback)

message(STATUS "We still don't have a main implementation of the player, so... TODO")
