#include "../decode/decoder.h"
#include "../decode/get.h"

#include "../memory/memory.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: `%s <song path>`\n", *argv);
        exit(1);
    }
	FILE *f = fopen(argv[1], "r");
    memory_ref mem(64000);
    fprintf(stderr, "Initial memory capacity is %d bytes\n", mem.cap());
	decoder *what = get_decoder(f, ENC_MP3);
    what->decode(mem);
    fprintf(stderr, "Final memory size is %d bytes\n", mem.cap());
return 0;
}
