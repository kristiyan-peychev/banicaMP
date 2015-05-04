#ifndef DECODER_BHIETHBX

#define DECODER_BHIETHBX

#include <cstdio>
#include <cstring>

class decoder {
public:
	decoder() { }
	virtual ~decoder(void) { }
	virtual bool decode(FILE *) = 0; // outfile
};

#include "FLAC_decoder.h"
#include "MPEG_decoder.h"


#endif /* end of include guard: DECODER_BHIETHBX */
