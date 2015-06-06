#include "../decode/MPEG_decoder.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

#define SONG_PATH "/falos/08 Throw Away.mp3"
#define OUT_PATH "/home/kawaguchi/test.wav"

int main(int argc, const char *argv[]) {
	FILE *file, *f;
	file = fopen(SONG_PATH, "r");
	f = fopen(OUT_PATH, "w");
	MPEG_decoder test(file);
	test.decode(f);
	/*
	fprintf(f, "TEST\n");
	char *p = (char *) malloc(1000 * sizeof(char));
	int c = 0;
	while (fread(p, 1, 1000, file)) {
		write(2, p, 1000);
		c++;
	}
	fprintf(stdout, "%d\n", c);
	*/
	fclose(f);
	fclose(file);
return 0;
}

