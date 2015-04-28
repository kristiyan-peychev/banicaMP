#ifndef MPEG_DECODER_XF7CBWKS

#define MPEG_DECODER_XF7CBWKS

#include "decoder.h"
#include <mad.h>

struct buffer {
	unsigned char *start;
	long length;
};

struct uberbuff {
	buffer *buf;
	FILE *f, *rd;
	uberbuff(buffer *b, FILE *ff, FILE *sf) :
				buf(b), f(ff), rd(sf) { }
};

class MPEG_decoder : public decoder {
	FILE *file; // input
public:
	MPEG_decoder(FILE *);
	~MPEG_decoder(void);
	bool decode(FILE *);
/*
private:
	enum mad_flow input(void *data, struct mad_stream *);
	enum mad_flow output(void *data, const struct mad_header *, struct mad_pcm *);
	enum mad_flow error(void *data, struct mad_stream *, struct mad_frame *);
*/
};

#endif /* end of include guard: MPEG_DECODER_XF7CBWKS */
