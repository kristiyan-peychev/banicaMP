#include "config.h"
#include <iostream>

int main(int argc, const char *argv[]) {
    config c("/home/kawaguchi/.banicaMP");
    std::cout << c << std::endl;
    c.set_last_vol(123);
    c.set_max_songs(1232222);
    c.set_min_songs(24);
    c.flush_conf_in_file("/home/kawaguchi/.banicaMP");
return 0;
}

