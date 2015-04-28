#include "MPEG_decoder.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, const char *argv[]) {
	FILE *file, *KOR;
	file = fopen("/falos/14 East.mp3", "r");
	KOR = fopen("/home/kawaguchi/test.wav", "w");
	/*
	int fd1 = open("/home/kawaguchi/14 East.mp3", O_RDONLY);
	int fd2 = open("/home/kawaguchi/test.wav", O_WRONLY);
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

