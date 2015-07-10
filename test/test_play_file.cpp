#include "../play.h"

// Change this to a wave file to test the play
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
    sleep(5);
    player->pause();
    fprintf(stderr, "lawl\n");
    player->play();
    sleep(5);
    fprintf(stderr, "seven\n");
    player->toggle_pause();
    player->toggle_pause();
    sleep(4);
    player->stop();
    t1.join();
return 0;
}
