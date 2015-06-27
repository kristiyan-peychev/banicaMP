#include <cstdio>
#include "../memory/memory.h"

int main(int argc, const char *argv[]) {
    memory mem(512);
    char *p = (char *) mem.begin();
    for (int i = 0; i < 0x100; i++)
        *(p + i) = i;

    for (int i = 0; i < 0x100; i++) {
        printf("%d\n", i);
    }
return 0;
}
