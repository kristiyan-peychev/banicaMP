cmake_minimum_required(VERSION 2.6)

project(playlist)
#cmake_policy(SET CMP0015 NEW)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")
set(SONG_DEPLIBS ${SONG_DEPLIBS} tag FLAC FLAC++ mad)
set(SONG_LOCALDEPLIBS ${SONG_LOCALDEPLIBS} ${PROJECT_SOURCE_DIR}/../play/linux/build/libplay.so ${PROJECT_SOURCE_DIR}/../../decode/build/libdecode.so)
#set(SONG_LOCALDEPLIBS ${SONG_LOCALDEPLIBS} /home/kawaguchi/banicaMP/playback/play/linux/build/libplay.so /home/kawaguchi/banicaMP/decode/build/libdecode.so)
#set(SONG_DEPLIBSLOC ${SONG_DEPLIBSLOC} "../../../decode/build" "../../play/linux/build")
# NOTE: From a build directory _ONLY_

add_executable(test_song playlist.cpp song.cpp ../../test/test_song.cpp) #??

set_property(TARGET test_song PROPERTY CXX_STANDARD 11)
target_link_libraries(test_song ${SONG_DEPLIBS})
#link_directories(${SONG_DEPLIBSLOC})
target_link_libraries(test_song ${SONG_LOCALDEPLIBS})
