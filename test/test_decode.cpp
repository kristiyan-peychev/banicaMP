#include "../decode/decoder.h"
#include "../decode/get.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: `%s <input song path> <output song path>`\n", *argv);
        exit(1);
    }
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        perror("fopen");
        exit(1);
    }
    decoder *what = get_decoder(f, ENC_MP3);
    FILE *ofile = fopen(argv[2], "w+");
    if (f == NULL) {
        perror("fopen");
        exit(1);
    }
    what->decode(ofile);
    return 0;
}
