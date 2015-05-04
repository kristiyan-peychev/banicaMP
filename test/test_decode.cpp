#include "../decode/decoder.h"
#include "../decode/get.h"
#include <cstdio>

#define KOR "/falos/14 East.mp3"
#define OKOR "/home/kawaguchi/newtest.wav"

int main(int argc, const char *argv[]) {
	FILE *file = fopen(KOR, "r");
	decoder *what = get_decoder(file, "MP3");
	FILE *ofile = fopen(OKOR, "w");
	what->decode(ofile);
return 0;
}
