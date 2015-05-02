#include "play.h"

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <cstdio>

alsa_wav_player::alsa_wav_player(void) { }

alsa_wav_player::alsa_wav_player(const alsa_wav_player &) { }

alsa_wav_player &alsa_wav_player::operator=(const alsa_wav_player &) { }

alsa_wav_player::alsa_wav_player(FILE *filep) : song(file), is_paused(true) { }

alsa_wav_player::~alsa_wav_player(void) { }

void alsa_wav_player::begin(void)
{
	pid_t cpid;
	char *pwd = get_current_dir_name(); // NOTE: this has to be freed
	// TODO finish this
}
