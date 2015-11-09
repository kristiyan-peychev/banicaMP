cmake_minimum_required(VERSION 2.8)

if (WIN32)
    add_subdirectory(windows windows/build)
elseif(UNIX)
    add_subdirectory(linux linux/build)
else()
    message(FATAL_ERROR "Platform not supported")
endif()

