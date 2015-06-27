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
    memory *mem;
    void *p;
	uberbuff(buffer *b, FILE *ff, FILE *sf, memory *m) :
				buf(b), f(ff), rd(sf), mem(m), p((m ? m->begin() : NULL)) { }
};

class MPEG_decoder final : public decoder {
	FILE *file; // input
public:
	MPEG_decoder(FILE *);
	~MPEG_decoder(void);
	bool decode(FILE *);
    bool decode(memory *);
public:
    MPEG_decoder(void) = delete;
    MPEG_decoder &operator=(const MPEG_decoder &) = delete;
};

#endif /* end of include guard: MPEG_DECODER_XF7CBWKS */
