#ifndef PLAY_J01D18YC

#define PLAY_J01D18YC

#include "../audio.h"

#include <unistd.h>
#include <cstdio>

#define NAME_SIZE 256
#define pwd "/home/kawaguchi/banicaMP/playback/audio/linux/build"

enum {
    FLG_FILE = 1,
    FLG_MEM = 2
};

class alsa_wav_player : public play_wav {
private:
	bool is_paused;
	pid_t childpid;
    int child_pipe;

	int filedsc;
    //memory *mem;
    bool flg;
public:
	explicit alsa_wav_player(FILE *);
    explicit alsa_wav_player(memory_ref &);
	explicit alsa_wav_player(int);
	~alsa_wav_player(void);
public:
	alsa_wav_player(void) = delete;
	alsa_wav_player(const alsa_wav_player &) = delete;
	alsa_wav_player &operator=(const alsa_wav_player &) = delete;
public:
	void begin(void);
	void play(void);
    void pause(void);
	void toggle_pause(void);
	void stop(void);
    void seek(int);
};

#endif /* end of include guard: PLAY_J01D18YC */
