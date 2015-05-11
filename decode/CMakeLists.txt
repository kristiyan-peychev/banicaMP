cmake_minimum_required(VERSION 2.6)

project (decode)
set (DEOCDE_DEPLIBS ${DEPLIBS} FLAC)
set (DEOCDE_DEPLIBS ${DEPLIBS} FLAC++)
set (DEOCDE_DEPLIBS ${DEPLIBS} mad)
add_library (decode SHARED FLAC_decoder.cpp MPEG_decoder.cpp get.cpp)
#target_link_libraries (decode DEOCDE_DEPLIBS)
