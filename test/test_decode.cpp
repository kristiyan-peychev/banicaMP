#include "../decode/decoder.h"
#include "../decode/get.h"
#include <cstdio>

#define file2 "/home/nikolay/31. Jinsei wa Belt Conveyor no Youni Nagareru.flac"
#define file "/home/nikolay/ACDC - Whole Lotta Rosie.mp3"
#define Ofile "/home/kawaguchi/amfg.wav"

#define fi "/falos/11 The Ulimate.mp3"

int main(int argc, const char *argv[]) {
	FILE *f = fopen(fi, "r");
	decoder *what = get_decoder(f, ENC_MP3);
	FILE *ofile = fopen(Ofile, "w+");
	what->decode(ofile);
return 0;
}
