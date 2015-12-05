cmake_minimum_required(VERSION 2.6)

project(playlist)
#cmake_policy(SET CMP0015 NEW)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-lpthread -std=c++14")
set(SONG_DEPLIBS ${SONG_DEPLIBS} tag FLAC FLAC++ mad asound rt m dl fftw3)
set(SONG_LOCALDEPLIBS ${SONG_LOCALDEPLIBS} ${PROJECT_SOURCE_DIR}/../../memory/build/libmem.so ${PROJECT_SOURCE_DIR}/../audio/linux/build/libaudio.so ${PROJECT_SOURCE_DIR}/../../decode/build/libdecode.so)
#set(SONG_LOCALDEPLIBS ${SONG_LOCALDEPLIBS} /home/kawaguchi/banicaMP/playback/play/linux/build/libplay.so /home/kawaguchi/banicaMP/decode/build/libdecode.so)
#set(SONG_DEPLIBSLOC ${SONG_DEPLIBSLOC} "../../../decode/build" "../../play/linux/build")
# NOTE: From a build directory _ONLY_

if (DEBUG)
    set(CMAKE_BUILD_TYPE Debug)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(test_song playlist.cpp song.cpp ../../config/config.cpp ../../test/test_playlist.cpp) #??

target_link_libraries(test_song ${SONG_DEPLIBS})
#link_directories(${SONG_DEPLIBSLOC})
target_link_libraries(test_song ${SONG_LOCALDEPLIBS})
