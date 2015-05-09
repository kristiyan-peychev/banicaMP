#ifndef GET_R1S6CD6W

#define GET_R1S6CD6W

#include "decoder.h"

// FIXME remove hardcode
extern "C" decoder *get_decoder(FILE *file, const char *enc_type)
{
	if (!strncmp(enc_type, "FLAC", sizeof("FLAC")))
		return new flac_decoder(file);
	else if (!strncmp(enc_type, "MP3", sizeof("MP3")))
		return new MPEG_decoder(file);
	else
		return NULL;
}

#endif /* end of include guard: GET_R1S6CD6W */
