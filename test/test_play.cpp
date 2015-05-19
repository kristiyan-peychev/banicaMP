#include "../playback/play/play.h"

#define WAV_FPATH "/home/nikolay/ffdf.wav"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int main(int argc, const char *argv[]) {
	FILE *fp = fopen(WAV_FPATH, "rb");
	play_wav *player = get_player(fp);
	player->begin();
return 0;
}
