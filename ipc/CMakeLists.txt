cmake_minimum_required(VERSION 2.8)

if (WIN32)
    add_subdirectory(windows windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_subdirectory(unix unix/build)
else()
    message(FATAL_ERROR "Platform not supported")
endif()

