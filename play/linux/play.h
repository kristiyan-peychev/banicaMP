#ifndef PLAY_J01D18YC

#define PLAY_J01D18YC

#include "../play.h"

class alsa_wav_player : public wav_play {
private:
	FILE *song;
	bool is_paused;
public:
	alsa_wav_player(FILE *);
	~alsa_wav_player(void);
	void begin(void);
	void play(void);
	void pause(void);
	void toggle_pause(void);
private:
	alsa_wav_player(void);
	alsa_wav_player(const alsa_wav_player &);
	alsa_wav_player &operator=(const alsa_wav_player &);
};

#endif /* end of include guard: PLAY_J01D18YC */
