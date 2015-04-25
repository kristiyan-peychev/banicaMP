#include "MPEG_decoder.h"

MPEG_decoder::MPEG_decoder(FILE *f)
{
	file = f;
}

MPEG_decoder::~MPEG_decoder(void)
{
	fclose(file);
}

bool MPEG_decoder::decode(FILE *f)
{
	return false;
}
