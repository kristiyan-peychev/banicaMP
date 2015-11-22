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
    memory_ref out_mem;
    uberbuff(buffer *b, FILE *ff, FILE *sf, memory_ref &mem) :
        buf(b), f(ff), rd(sf), out_mem(mem) { }
};

class MPEG_decoder final : public decoder {
    FILE *file; // input
public:
    MPEG_decoder(FILE *);
    ~MPEG_decoder(void);
    bool decode(FILE *);
    bool decode(memory_ref &);
public:
    MPEG_decoder(void)                              = delete;
    MPEG_decoder &operator=(const MPEG_decoder &)   = delete;
};

#endif /* end of include guard: MPEG_DECODER_XF7CBWKS */
