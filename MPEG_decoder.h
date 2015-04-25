#ifndef MPEG_DECODER_XF7CBWKS

#define MPEG_DECODER_XF7CBWKS

#include "decoder.h"

class MPEG_decoder : public decoder {
	FILE *file;
public:
	MPEG_decoder(FILE *);
	~MPEG_decoder(void);
	bool decode(FILE *);
};

#endif /* end of include guard: MPEG_DECODER_XF7CBWKS */
