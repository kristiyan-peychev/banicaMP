#include "../decode/decoder.h"
#include "../decode/get.h"

#include "../memory/memory.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, const char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: `%s <song path>`\n", *argv);
    }
	FILE *f = fopen(argv[1], "r");
    memory_ref mem(640000000);
	decoder *what = get_decoder(f, ENC_MP3);
    what->decode(&mem);
return 0;
}
