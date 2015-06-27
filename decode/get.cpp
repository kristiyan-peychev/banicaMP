#include <cstring>
#include "get.h"
#include "decoder.h"

//extern "C" decoder *get_decoder(FILE *file, const char *enc_type)
//{
	//if (!strncmp(enc_type, "FLAC", sizeof("FLAC")))
		//return new flac_decoder(file);
	//else if (!strncmp(enc_type, "MP3", sizeof("MP3")))
		//return new MPEG_decoder(file);
	//else
		//return NULL;
//}

extern "C" decoder *get_decoder(FILE *file, enum encodings enc)
{
    if (enc == ENC_FLAC)
		return new flac_decoder(file);
    else if (enc == ENC_MP3)
        return new MPEG_decoder(file);
    else
        return NULL;
}

