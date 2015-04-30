#ifndef DECODER_BHIETHBX

#define DECODER_BHIETHBX

#include <cstdio>

class decoder {
public:
	decoder() { }
	virtual ~decoder(void) { }
	virtual bool decode(FILE *) = 0; // outfile
};

#endif /* end of include guard: DECODER_BHIETHBX */
