#include "../playback/playlist/song.h"
#include <cstdio>


#define MP3_SONG_PATH "/home/nikolay/ACDC - Whole Lotta Rosie.mp3"
#define FLAC_SONG_PATH "/home/nikolay/31. Jinsei wa Belt Conveyor no Youni Nagareru.flac"

#include <iostream>
using namespace std;

int main(int argc, const char *argv[]) {
	song mp3(MP3_SONG_PATH);
    song flac(FLAC_SONG_PATH);
    printf("%s\n", flac.get_path());
    printf("%s\n", mp3.get_encoding());
    printf("%s\n", flac.get_encoding());
    cout<<flac.get_info().title<<endl;
    flac.start();
   // printf("%s\n", mp3.get_info()->tag->title().toCString(true));

    return 0;    
}

