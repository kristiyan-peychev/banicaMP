#include "../decode/decoder.h"
#include "../decode/get.h"

#include "../memory/memory.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, const char *argv[]) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: `%s <song path> [<filedump>]`\n", *argv);
        exit(1);
    }
	FILE *f = fopen(argv[1], "r");
    memory_ref mem(64000);
    fprintf(stderr, "Initial memory capacity is %d bytes\n", mem.cap());
	decoder *what = get_decoder(f, ENC_MP3);
    what->decode(mem);
    fprintf(stderr, "Final memory size is %d bytes\n", mem.cap());
    if (argc == 3) {
        FILE *file = fopen(argv[2], "wb+");
        fwrite(mem.begin(), 1, mem.cap(), file);
    }
return 0;
}
