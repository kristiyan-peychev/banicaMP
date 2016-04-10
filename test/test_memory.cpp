#include <cstdio>
#include "../memory/memory.h"
#include <cstring>
#include <ctime>

const char testing[] = "111111111111111111111111";

int main(int argc, const char *argv[]) {
    shared_memory mem = memory::alloc(0x100);
    fprintf(stderr, "Allocated %d bytes\n", mem->cap());

    char dest_[32] = {};
    char *dest = (char *) dest_;

    fprintf(stderr, "Testing expand w/ write/read\n");
    {
        clock_t start = clock();
        for (int i = 0; i < 0x100000; i++) {
            mem->write(testing, sizeof(testing) - 1);
        }
        for (int i = 0; i < 0x100000; i++) {
            int ff = mem->read(&dest, sizeof(testing) - 1);
            if (strcmp(dest, testing)) {
                fprintf(stderr, "Error: strings `%s` and `%s` are different at %d\n", i);
                exit(1);
            }
        }
        fprintf(stderr, "Expand test finished for %fs\n", float(clock() - start) / CLOCKS_PER_SEC);
    }
    fprintf(stderr, "Test succeeded\n");
return 0;
}
