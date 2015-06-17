#include "play.h"

win_wav_player::win_wav_player(FILE *fp) :
        file(fp), playback_thread_handle()
{
    // TODO: actually add the callbacks

    WAVEFORMATEX format = {WAVE_FORMAT_PCM, 2, 44100, 44100, 8/*?*/, 0};
    waveOutOpen((LPHWAVEOUT) &sound_handle, WAVE_MAPPER, 
            &format, (DWORD_PTR) NULL, (DWORD_PTR) NULL, 
            CALLBACK_NULL);
}

win_wav_player::~win_wav_player(void)
{
    waveOutClose(handle);
}

win_wav_player::playback_thread_fn(win_wav_player * const player)
{
    // FIXME
    char *buffer = (char *) malloc(BUFF_SIZE * sizeof(*buffer));

    WAVEHDR song;
    song.lpData = buffer;
    song.dwBufferLength = BUFF_SIZE;

    player->paused.unlock();
    while (fread(buffer, 1, BUFF_SIZE, player->file)) {
        player->paused.lock();
        waveOutWrite(player->handle, (LPWAVEHDR) &song, sizeof(song));
        player->paused.unlock();
    }

    free(buffer);
}

void win_wav_player::begin(void)
{
    std::thread tmp(playback_thread_fn, this);
    thread = tmp;
}

void win_wav_player::stop(void)
{
    waveOutReset(handle);
    paused.lock();
    // I think
}

void win_wav_player::play(void)
{
    waveOutRestart(handle);
    paused.unlock();
}

void win_wav_player::pause(void)
{
    paused.lock();
    waveOutPause(handle);
}

void win_wav_player::toggle_pause(void)
{
    if (!paused.try_lock()) {
        waveOutRestart(handle);
        paused.unlock();
    } else {
        waveOutPause(handle);
    }
}

void win_wav_player::seek(void)
{
    // TODO: there actually _is_ an api function
    // for doing exactly this, but I have to test
    // everything else first. So I'm leaving it for later
}

