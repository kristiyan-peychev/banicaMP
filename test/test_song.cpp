#include "../playback/playlist/song.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>


#define MP3_SONG_PATH "/home/nikolay/ACDC - Whole Lotta Rosie.mp3"
#define FLAC_SONG_PATH "/home/nikolay/31. Jinsei wa Belt Conveyor no Youni Nagareru.flac"


int main(int argc, const char *argv[]) {
	song mp3(MP3_SONG_PATH);
    song* flac = new song(FLAC_SONG_PATH);
    flac->start();
    sleep(5);
    flac->pause();
    sleep(2);
    flac->pause();
    sleep(5);
    flac->stop();
    delete flac;
    mp3.start();
    sleep(20);
    mp3.stop();

    return 0;    
}

