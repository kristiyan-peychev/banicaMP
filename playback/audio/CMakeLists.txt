cmake_minimum_required(VERSION 2.8)

project(audio)

set(AUDIO_INCLUDE_PATH "\"portaudio/audio.h\"")
configure_file("${PROJECT_SOURCE_DIR}/audio.h.in" "${PROJECT_SOURCE_DIR}/audio.h")
add_subdirectory(linux linux/build)

