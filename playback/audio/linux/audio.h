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
    int input_fd;
    memory_ref input_mem;
    bool flg;
public:
    ~alsa_wav_player(void);
    explicit alsa_wav_player(FILE *input);
    explicit alsa_wav_player(memory_ref &input);
    explicit alsa_wav_player(int input_fd);
public:
    alsa_wav_player(void)                               = delete;
    alsa_wav_player(const alsa_wav_player &)            = delete;
    alsa_wav_player &operator=(const alsa_wav_player &) = delete;
public:
    void begin(void); // start actual playback
    void play(void); // resume after pause or stop
    void pause(void); // pause the playback
    void toggle_pause(void); // toggle pause and unpause
    void stop(void); // stop playback
    void seek(int miliseconds); // seek through the song
private:
    void begin_mem (void);
    void begin_file(void);
};

#endif /* end of include guard: PLAY_J01D18YC */
