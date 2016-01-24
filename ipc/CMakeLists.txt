cmake_minimum_required(VERSION 2.8)


if (WIN32)
    add_subdirectory(windows windows)
    set(INCLUDE_DIR "\"${PROJECT_SOURCE_DIR}/windows/interprocess.h\"")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_subdirectory(unix unix/build)
    set(INCLUDE_DIR "\"${PROJECT_SOURCE_DIR}/unix/interprocess.h\"")
else()
    message(FATAL_ERROR "Platform not supported")
endif()


configure_file("${PROJECT_SOURCE_DIR}/get.h.in" "${PROJECT_SOURCE_DIR}/get.h")
