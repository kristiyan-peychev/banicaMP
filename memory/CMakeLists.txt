cmake_minimum_required(VERSION 2.8)

if(TEST_LEL)
    message(ISSSET)
else()
    message(NOSSET)
endif()
project (mem)
# TODO check for windows
set (MEM_SRC memory_linux.cpp)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11)

add_library (mem SHARED ${MEM_SRC})

