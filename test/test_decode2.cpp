#include "../decode/decoder.h"
#include "../decode/get.h"

#include "../memory/memory.h"
#include <cstdio>

#define file2 "/home/nikolay/31. Jinsei wa Belt Conveyor no Youni Nagareru.flac"
#define file "/home/nikolay/ACDC - Whole Lotta Rosie.mp3"
#define Ofile "/home/nikolay/ffdf.wav"
#define fi "/falos/Gintama/Gintama OST 2/31. Kamagata Eiichi - Jinsei wa Belt Conveyor no Youni Nagareru.flac"

int main(int argc, const char *argv[]) {
	FILE *f = fopen(fi, "r");
    memory mem(640000000);
	decoder *what = get_decoder(f, ENC_FLAC);
	//what->decode(ofile);
    what->decode(&mem);
return 0;
}
