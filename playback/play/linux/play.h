#ifndef PLAY_J01D18YC

#define PLAY_J01D18YC

#include "../play.h"

#include <unistd.h>
#include <cstdio>

#define NAME_SIZE 256

const char *aplay_args[] = {
	"/aplay", // executable name
	"-f", "cd", // 16-bit signed integer
	"-t", "wav", // WAVE file
	"-q", // quiet mode
	"-" // read from stdin
};

class alsa_wav_player : public play_wav {
private:
	bool is_paused;
	int filedsc;
	pid_t childpid;
public:
	alsa_wav_player(FILE *);
	explicit alsa_wav_player(int);
	~alsa_wav_player(void);
public:
	void begin(void);
	void play(void);
	void pause(void);
	void toggle_pause(void);
	void stop(void);
private:
	alsa_wav_player(void);
	alsa_wav_player(const alsa_wav_player &);
	alsa_wav_player &operator=(const alsa_wav_player &);
};

#endif /* end of include guard: PLAY_J01D18YC */