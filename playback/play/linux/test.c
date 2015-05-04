#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define f "/home/kawaguchi/banicaMP/playback/play/linux/aplay"

int main(int argc, const char *argv[]) {
	char **ff = (char **) malloc(2 * sizeof(char *)); 
	*ff = (char *) malloc(sizeof(f) * sizeof(char)); 
	strcpy(*ff, f);
	*(ff + 1) = 0;
	execv(*ff, ff);
	perror("execv");
	exit(1);
return 0;
}
