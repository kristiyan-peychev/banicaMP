cmake_minimum_required(VERSION 2.8)

project(audio)

if (WIN32)
    set(AUDIO_INCLUDE_PATH "\"windows/audio.h\"")
    configure_file("${PROJECT_SOURCE_DIR}/audio.h.in" "${PROJECT_SOURCE_DIR}/audio.h")
    add_subdirectory(windows windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(AUDIO_INCLUDE_PATH "\"linux/audio.h\"")
    configure_file("${PROJECT_SOURCE_DIR}/audio.h.in" "${PROJECT_SOURCE_DIR}/audio.h")
    add_subdirectory(linux linux/build)
else()
    message(FATAL_ERROR "Platform not supported")
endif()

