#ifndef FLAC_DECODER_PEOMBWH

#define FLAC_DECODER_PEOMBWH

#include "decoder.h"
#include "FLAC++/decoder.h"

class flac_decoder final : public decoder, public FLAC::Decoder::File {
public:
	flac_decoder(FILE *);
	~flac_decoder(void);
	bool decode(FILE *);
    bool decode(memory *);
private:
	::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *, const FLAC__int32 * const []);
	::FLAC__StreamDecoderWriteStatus mem_write_callback(const ::FLAC__Frame *, const FLAC__int32 * const []);
	::FLAC__StreamDecoderWriteStatus file_write_callback(const ::FLAC__Frame *, const FLAC__int32 * const []);
	void metadata_callback(const ::FLAC__StreamMetadata *);
	void error_callback(::FLAC__StreamDecoderErrorStatus);
public:
	flac_decoder(const flac_decoder &) = delete;
	flac_decoder &operator=(const flac_decoder &) = delete;
private:
	FILE *file; // input file

    bool memflg;
	FILE *f; // outfile
    memory *out;
    void *p;

	FLAC__uint64 total_samples;
	unsigned sample_rate;
	unsigned channels;
	unsigned bps;
};

#endif /* end of include guard: FLAC_DECODER_PEOMBWH */
