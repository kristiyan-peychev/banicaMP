#ifndef DECODER_BHIETHBX

#define DECODER_BHIETHBX

#include <cstdio>
#include <cstring>

#if defined(linux)
#define _LINUX
#elif defined(WIN32)
#define _WINDOWS
#endif
#include "../memory/memory.h"


class decoder {
public:
	decoder() { }
	virtual ~decoder(void) { }
	virtual bool decode(FILE *) = 0; // outfile
    virtual bool decode(memory *) = 0;
};

#include "FLAC_decoder.h"
#include "MPEG_decoder.h"


#endif /* end of include guard: DECODER_BHIETHBX */
