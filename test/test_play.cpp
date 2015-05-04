#include "../playback/play/play.h"

#define WAV_FPATH "/home/kawaguchi/test.wav"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main(int argc, const char *argv[]) {
	FILE *fp = fopen(WAV_FPATH, "r");
	play_wav *player = get_player(fp);
	player->begin();
	sleep(10);
	player->pause();
	sleep(5);
	player->pause();
	player->play();
	sleep(5);
	player->toggle_pause();
	player->toggle_pause();
return 0;
}
