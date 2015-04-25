#ifndef FLAC_DECODER_PEOMBWH

#define FLAC_DECODER_PEOMBWH

#include "decoder.h"
#include "FLAC++/decoder.h" // FIXME to local header

// TODO: test

class flac_decoder : public decoder, public FLAC::Decoder::File {
public:
	flac_decoder(FILE *);
	~flac_decoder(void);
	bool decode(FILE *);
private:
	::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame *, const FLAC__int32 * const []);
	void metadata_callback(const ::FLAC__StreamMetadata *);
	void error_callback(::FLAC__StreamDecoderErrorStatus);
private:
	flac_decoder(const flac_decoder &);
	flac_decoder &operator=(const flac_decoder &);
private:
	FILE *file;
	FILE *f; // outfile
	FLAC__uint64 total_samples;
	unsigned sample_rate;
	unsigned channels;
	unsigned bps;
};
#endif /* end of include guard: FLAC_DECODER_PEOMBWH */
