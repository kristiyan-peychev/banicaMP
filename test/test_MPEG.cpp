#include "..decode/MPEG_decoder.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

#define SONG_PATH "/falos/08 Throw Away.mp3"
#define OUT_PATH "/home/kawaguchi/test.wav"

int main(int argc, const char *argv[]) {
	FILE *file, *KOR;
	file = fopen(SONG_PATH, "r");
	KOR = fopen(OUT_PATH, "w");
	/*
	int fd1 = open(SONG_PATH, O_RDONLY);
	int fd2 = open(OUT_PATH, O_WRONLY);
	file = fdopen(fd1, "r");
	KOR = fdopen(fd2, "w");
	*/
	MPEG_decoder test(file);
	test.decode(KOR);
	/*
	fprintf(KOR, "TEST\n");
	char *p = (char *) malloc(1000 * sizeof(char));
	int c = 0;
	while (fread(p, 1, 1000, file)) {
		write(2, p, 1000);
		c++;
	}
	fprintf(stdout, "%d\n", c);
	*/
	fclose(KOR);
	fclose(file);
return 0;
}

