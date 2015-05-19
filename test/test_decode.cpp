#include "../decode/decoder.h"
#include "../decode/get.h"
#include <cstdio>

#define KOR2 "/home/nikolay/31. Jinsei wa Belt Conveyor no Youni Nagareru.flac"
#define KOR "/home/nikolay/ACDC - Whole Lotta Rosie.mp3"
#define OKOR "/home/nikolay/ffdf.wav"

int main(int argc, const char *argv[]) {
	FILE *file = fopen(KOR, "r");
	decoder *what = get_decoder(file, "MP3");
	FILE *ofile = fopen(OKOR, "w+");
	what->decode(ofile);
return 0;
}
