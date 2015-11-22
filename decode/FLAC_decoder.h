#ifndef FLAC_DECODER_PEOMBWH

#define FLAC_DECODER_PEOMBWH

#include "decoder.h"
#include "FLAC++/decoder.h"

class flac_decoder final : public decoder, public FLAC::Decoder::File {
public:
    ~flac_decoder(void);
    flac_decoder(FILE *);
public:
    bool decode(FILE *);
    bool decode(memory_ref &);
private:
    ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *, const FLAC__int32 * const []);
    ::FLAC__StreamDecoderWriteStatus mem_write_callback(const ::FLAC__Frame *, const FLAC__int32 * const []);
    ::FLAC__StreamDecoderWriteStatus file_write_callback(const ::FLAC__Frame *, const FLAC__int32 * const []);
    void metadata_callback(const ::FLAC__StreamMetadata *);
    void error_callback(::FLAC__StreamDecoderErrorStatus);
public:
    flac_decoder(const flac_decoder &)              = delete;
    flac_decoder &operator=(const flac_decoder &)   = delete;
private:
    FILE *file; // input file
private:
    bool memflg;
    FILE *f; // outfile
    memory_ref out_mem; // outmem
    void *p;
private:
    FLAC__uint64 total_samples;
    unsigned sample_rate;
    unsigned channels;
    unsigned bps;
private:
    ::FLAC__StreamDecoderWriteStatus (flac_decoder::*m_write_callback)(const ::FLAC__Frame *, const FLAC__int32 * const []);
};

#endif /* end of include guard: FLAC_DECODER_PEOMBWH */
