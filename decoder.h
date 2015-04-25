#ifndef DECODER_BHIETHBX

#define DECODER_BHIETHBX

#include <cstdio>

class decoder {
protected:
	FILE *file;
public:
	decoder();
	virtual ~decoder(void);
	virtual bool decode(FILE *) = 0; // Output file
};

#endif /* end of include guard: DECODER_BHIETHBX */
