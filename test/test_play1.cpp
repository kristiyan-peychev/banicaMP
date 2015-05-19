#include "../play.h"

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
	player->pause();
    fprintf(stderr, "HUH?\n");
	sleep(50);
	//sleep(5);
	//player->pause();
	//player->play();
	//sleep(5);
	//player->toggle_pause();
	//player->toggle_pause();
return 0;
}
