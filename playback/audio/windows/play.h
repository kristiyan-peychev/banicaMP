#ifndef PLAY_TX6K71RU

#define PLAY_TX6K71RU

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>

#include <cstdio>
#include <thread>
#include <mutex>

#include "../play.h"

class win_wav_player final : public play_wav {
    FILE *file;
    HWAVEOUT sound_handle;
    std::mutex paused;
    std::thread playback_thread_handle;
private:
    static void playback_thread_fn(win_wav_player *);
public:
    win_wav_player(FILE *);
    ~win_wav_player(void);
public:
    win_wav_player(void) = delete;
    win_wav_player(const win_wav_player &) = delete;
    win_wav_player &operator=(const win_wav_player &) = delete;
public:
    void begin(void) override;
    void play(void) override;
    void pause(void) override;
    void toggle_pause(void) override;
    void stop(void) override;
    void seek(int) override;
};

#endif /* end of include guard: PLAY_TX6K71RU */
