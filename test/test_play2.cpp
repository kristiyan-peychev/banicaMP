#include "../playback/play/play.h"

#define WAV_FPATH "/home/kawaguchi/test.wav"

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <utility>

void f(play_wav *player)
{
    player->begin();
    fprintf(stderr, "PLAYER EXITED\n");
}

int main(int argc, const char *argv[]) {
	FILE *fp = fopen(WAV_FPATH, "r");
	play_wav *player = get_player(fp);
	//player->begin();
    std::thread t1(f, player);
    fprintf(stderr, "Begin?\n");
	sleep(10);
    fprintf(stderr, "wtf?\n");
    player->seek(9900000);
    fprintf(stderr, "Seek ret\n");
	sleep(10);
    player->stop();
    fprintf(stderr, "Stop ret\n");
    t1.join();
    std::thread t2(f, player);
    sleep(10);
    player->stop();
    t2.join();
return 0;
}
