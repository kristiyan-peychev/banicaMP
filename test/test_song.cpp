#include "../playback/playlist/song.h"
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#define MP3_SONG_PATH2 "/media/win/Music/Avengers OST/14 - Pistols At Dawn .mp3"
#define MP3_SONG_PATH "/home/nikolay/ACDC - Whole Lotta Rosie.mp3"
#define FLAC_SONG_PATH "/home/nikolay/31. Jinsei wa Belt Conveyor no Youni Nagareru.flac"

const int T = 5;

int main(int argc, const char *argv[]) {
	song mp3(MP3_SONG_PATH2);
    song* flac = new song(FLAC_SONG_PATH);
    flac->start();
    sleep(T);
    flac->pause();
    sleep(T);
    flac->pause();
    flac->seek(30);
    sleep(T);
    flac->stop();
    delete flac;
    mp3.start();
    sleep(T);
    mp3.stop();
    sleep(T);

    return 0;    
}

