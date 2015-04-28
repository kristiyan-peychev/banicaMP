#include "MPEG_decoder.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

#define BUFF_SIZE 32767

/*
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
*/

static int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
			sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
			sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

MPEG_decoder::MPEG_decoder(FILE *lol)
{
	file = lol;
}

MPEG_decoder::~MPEG_decoder(void)
{
	//fclose(file);
}

static std::string LOLDIS;

static enum mad_flow input(void *data,
				struct mad_stream *stream)
{
	uberbuff *buf = static_cast<uberbuff *>(data);
	int what;

	if ((what = fread(buf->buf->start, 1, buf->buf->length, buf->rd)) > 0) {
		mad_stream_buffer(stream, buf->buf->start, buf->buf->length);
		return MAD_FLOW_CONTINUE;
	} else {
		fprintf(stderr, "Error %d\n", what);
		return MAD_FLOW_STOP;
	}
	return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data, 
					struct mad_stream *stream,
					struct mad_frame *frame)
{
	uberbuff *buf = static_cast<uberbuff *>(data);

	//TODO throw
	fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n", stream->error, mad_stream_errorstr(stream), stream->this_frame - buf->buf->start);
	return MAD_FLOW_CONTINUE;
}

static enum mad_flow output(void *data, 
					const struct mad_header *header,
					struct mad_pcm *pcm)
{
	signed int sample;
	unsigned nchannels, nsamples;
	const mad_fixed_t *left_ch, *right_ch;
	uberbuff *buf = static_cast<uberbuff *>(data);

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	while (nsamples--) {

		/* output sample(s) in 16-bit signed little-endian PCM */

		sample = scale(*left_ch++);
		putc((sample >> 0) & 0xff, buf->f);
		putc((sample >> 8) & 0xff, buf->f);

		if (nchannels == 2) {
			sample = scale(*right_ch++);
			putc((sample >> 0) & 0xff, buf->f);
			putc((sample >> 8) & 0xff, buf->f);
		}
	}
	
	return MAD_FLOW_CONTINUE;
}

bool MPEG_decoder::decode(FILE *lol)
{
	buffer buf;
	struct mad_decoder dec;
	uberbuff pro_buff(&buf, lol, file);
	int result;

	//buf.start = new unsigned char [BUFF_SIZE];
	buf.start = (unsigned char *) malloc(BUFF_SIZE * sizeof(*buf.start)); 
	buf.length = BUFF_SIZE;

	mad_decoder_init(&dec, &pro_buff, input, 0, 0, 
			output, error, 0);

	result = mad_decoder_run(&dec, MAD_DECODER_MODE_SYNC);

	mad_decoder_finish(&dec);
	free(buf.start);
	
	// FIXME
	return static_cast<bool>(!!result);
}

