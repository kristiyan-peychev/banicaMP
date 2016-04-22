#ifndef AUDIO_VS0W8UG0

#define AUDIO_VS0W8UG0

#include "../audio.h"
#include "stream.h"
#include "device_manager.h"

#include <portaudio.h>
#include <exception>

#define FRAMES_PER_BUFFER 100

typedef enum {
    state_none        = 0,
    state_initialized = 1,
    state_stopped     = 2,
    state_playing     = 4,
    state_paused      = 8,
} player_state;

class portaudio_wav_player : public play_wav {
protected:
    PaError             global_error;
    PaStreamParameters  input_stream_params;
    PaStreamParameters  output_stream_params;
    PaStream           *main_stream;
    PaDeviceIndex       device_index;
    int                 channel_count;
    double              sample_rate;

    long                state_flags;

    stream              read_stream;
    long                frames_last_read;

    decoder            *active_decoder;
    int                 frames_per_buffer;
public:
   ~portaudio_wav_player();
    portaudio_wav_player(FILE *source,
                         int channels,
                         decoder *decoder = NULL,
                         int playback_device_index = -1,
                         double sample_rate = 44100.0f);
    portaudio_wav_player(shared_memory source,
                         int channels,
                         decoder *decoder = NULL,
                         int playback_device_index = -1,
                         double sample_rate = 44100.0f);
    portaudio_wav_player(std::fstream &&source,
                         int channels,
                         decoder *decoder = NULL,
                         int playback_device_index = -1,
                         double sample_rate = 44100.0f);

    void initialize();

//====================================================
// play_wav methods
public:
    void begin(void) override;
    void play(void) override;
    void pause(void) override;
    void toggle_pause(void) override;
    void stop(void) override;
    void seek(int bytes) override;
public:
    bool get_playing(void) const override;
    bool get_paused(void) const override;
//====================================================
public:
    bool get_flag(int mask) const;

    long fill_buffer(char **buffer, size_t frame_count);
};

namespace audio {
    class exception : public std::exception { };

    class initialization_failed : public exception {
        const char *what()  const noexcept  {
            return "PortAudio not initialized";
        }
    };

    class playback_could_not_start : public exception {
        const char *what()  const noexcept  {
            return "PortAudio stream failed to start";
        }
    };

    class playback_could_not_stop : public exception {
        const char *what()  const noexcept  {
            return "PortAudio stream failed to stop";
        }
    };

    class double_initialization : public exception {
        const char *what()  const noexcept  {
            return "PortAudio device manager singleton attempt to make a second instance";
        }
    };

    class player_failed_to_register : public exception {
        const char *what()  const noexcept  {
            return "Player failed to register with the portaudio device manager";
        }
    };

    class wav_player_not_initialized : public exception {
        const char *what()  const noexcept  {
            return "Portaudio wav player was not initialized";
        }
    };
}

#endif /* end of include guard: AUDIO_VS0W8UG0 */
