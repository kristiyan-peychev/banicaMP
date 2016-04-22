cmake_minimum_required(VERSION 2.6)

project(portable_audio_player)
set(APLAY_DEPLIBS ${PORTAUDIO_PLAYER_DEPLIBS} pthread asound rt m dl fftw3 portaudio)
set(APLAY_PWD "\"${PROJECT_BINARY_DIR}\"")
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

if (DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(audio audio.cpp device_manager.cpp stream.cpp)
target_link_libraries(audio ${APLAY_DEPLIBS})
