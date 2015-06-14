#include "config.h"
#include <cstdlib>
#include <algorithm>

#if defined(_WIN32)
#include <io.h>
auto access_funct(const char *fname) -> bool
{
    return _access(fname, 02);
}
#else
#include <unistd.h>
#include <fcntl.h>
auto access_funct(const char *fname) -> bool
{
    return access(fname, R_OK);
}
#endif

config::config(void) : version(CURRENT_CONFIG_VERSION), max_decoded_songs(2),
        min_decoded_songs(1), last_volume(0)
{
    std::for_each(filters, filters + FILTERS_SIZE, 
            [](float &f) { f = 0; });
}

config::config(const char *file) : version(CURRENT_CONFIG_VERSION), max_decoded_songs(2),
        min_decoded_songs(1), last_volume(0)
{
    FILE *filep;

    if ((access_funct(file) == -1) || (filep = fopen(file, "rb")) == NULL) {
        fprintf(stderr, "Error opening file, defaulting\n");
        return;
    }

    fread(this, sizeof(*this), 1, filep);
    fclose(filep);

    // TODO: implement old version support
    if (version < CURRENT_CONFIG_VERSION)
        fprintf(stderr, "Warning: reading an old version file\n");
}

void config::flush_conf_in_file(const char *file) const throw()
{
    FILE *filep = fopen(file, "wb");

    if (filep == NULL)
        throw fopen_except();

    fwrite(this, sizeof(*this), 1, filep);
    fclose(filep);
}

std::ostream &operator<<(std::ostream &ostr, const config &c)
{
    ostr << "Config file version "  << c.version << 
            ". Min/max allowed decoded songs: " << c.max_decoded_songs <<  " " << 
            c.min_decoded_songs;
    return ostr;
}

