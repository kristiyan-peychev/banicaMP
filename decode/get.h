#ifndef GET_R1S6CD6W

#define GET_R1S6CD6W

#include "decoder.h"

enum encodings {
    ENC_FLAC,
    ENC_MP3
};

// FIXME remove hardcode
//extern "C" decoder *get_decoder(FILE *file, const char *encoding);
extern "C" decoder *get_decoder(FILE *file, enum encodings);

#endif /* end of include guard: GET_R1S6CD6W */
