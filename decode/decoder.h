#ifndef DECODER_BHIETHBX

#define DECODER_BHIETHBX

#include <cstdio>
#include <cstring>
#include <exception>

#include "../memory/memory.h"

class decoder {
public:
    decoder() { }
    virtual ~decoder(void) { }
    virtual bool decode(FILE *output) = 0;
    virtual bool decode(shared_memory output) = 0;
};

namespace decode {
    class exception : public std::exception {
    };
}

#include "FLAC_decoder.h"
#include "MPEG_decoder.h"

#endif /* end of include guard: DECODER_BHIETHBX */
