#include "../decode/decoder.h"
#include "../decode/get.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: `%s <song path>`\n", *argv);
        exit(1);
    }
	FILE *f = fopen(argv[1], "r");
	FILE *f = fopen(fi, "r");
	decoder *what = get_decoder(f, ENC_MP3);
	FILE *ofile = fopen(Ofile, "w+");
	what->decode(ofile);
return 0;
}
