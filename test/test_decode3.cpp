#include "../decode/decoder.h"
#include "../decode/get.h"

#include "../memory/memory.h"
#include <cstdio>

#define fi "/falos/11 The Ulimate.mp3"

int main(int argc, const char *argv[]) {
	FILE *f = fopen(fi, "r");
    memory mem(640000000);
	decoder *what = get_decoder(f, ENC_MP3);
	//what->decode(ofile);
    what->decode(&mem);
return 0;
}
