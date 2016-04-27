#include "audio.h"

#include <cstring>
#include <math.h>

namespace audio {
    portaudio_manager device_manager;
}

static int pa_stream_callback(const void *input_buffer,
                              void *output_buffer,
                              unsigned long frames_per_buffer,
                              const PaStreamCallbackTimeInfo *time_info,
                              PaStreamCallbackFlags status_flags,
                              void *context)
{
    (void) input_buffer; (void) time_info; (void) status_flags;

    portaudio_wav_player *player = reinterpret_cast<portaudio_wav_player *>(context);
    player->fill_buffer((char **) &output_buffer, frames_per_buffer);

    return 0;
}

portaudio_wav_player::~portaudio_wav_player()
{
    if (!get_flag(state_initialized))
        return;
    if (get_flag(state_playing))
        stop();

    audio::device_manager.unreg(this);
}

portaudio_wav_player::portaudio_wav_player(FILE *source, int channels, decoder *decoder,
        int playback_device_index, double sample_rate)
: channel_count(channels)
, sample_rate(sample_rate)
, state_flags(state_none)
, read_stream(source)
, frames_last_read(0)
, active_decoder(decoder)
, frames_per_buffer(FRAMES_PER_BUFFER)
{
    device_index = playback_device_index;

    memset(&input_stream_params, 0, sizeof(input_stream_params));

    output_stream_params.channelCount = channel_count;

    if (playback_device_index < 0 ||
            playback_device_index > Pa_GetDeviceCount())
        device_index = Pa_GetDefaultOutputDevice();
    output_stream_params.device = device_index;

    output_stream_params.hostApiSpecificStreamInfo = NULL;
    output_stream_params.sampleFormat = paInt16;
    output_stream_params.suggestedLatency =
            Pa_GetDeviceInfo(device_index)->defaultLowOutputLatency;
}

portaudio_wav_player::portaudio_wav_player(shared_memory source, int channels, decoder *decoder,
        int playback_device_index, double sample_rate)
: channel_count(channels)
, sample_rate(sample_rate)
, state_flags(state_none)
, read_stream(source)
, frames_last_read(0)
, active_decoder(decoder)
, frames_per_buffer(FRAMES_PER_BUFFER)
{
    device_index = playback_device_index;

    memset(&input_stream_params, 0, sizeof(input_stream_params));

    output_stream_params.channelCount = channel_count;

    if (playback_device_index < 0 ||
            playback_device_index > Pa_GetDeviceCount())
        device_index = Pa_GetDefaultOutputDevice();
    output_stream_params.device = device_index;

    output_stream_params.hostApiSpecificStreamInfo = NULL;
    output_stream_params.sampleFormat = paInt16;
    output_stream_params.suggestedLatency =
            Pa_GetDeviceInfo(device_index)->defaultLowOutputLatency;
}

portaudio_wav_player::portaudio_wav_player(std::fstream &&source, int channels, decoder *decoder,
        int playback_device_index, double sample_rate)
: channel_count(channels)
, sample_rate(sample_rate)
, state_flags(state_none)
, read_stream(std::move(source))
, frames_last_read(0)
, active_decoder(decoder)
, frames_per_buffer(FRAMES_PER_BUFFER)
{
    device_index = playback_device_index;

    memset(&input_stream_params, 0, sizeof(input_stream_params));

    output_stream_params.channelCount = channel_count;

    if (playback_device_index < 0 ||
            playback_device_index > Pa_GetDeviceCount())
        device_index = Pa_GetDefaultOutputDevice();
    output_stream_params.device = device_index;

    output_stream_params.hostApiSpecificStreamInfo = NULL;
    output_stream_params.sampleFormat = paInt16;
    output_stream_params.suggestedLatency =
            Pa_GetDeviceInfo(device_index)->defaultLowOutputLatency;
}

void portaudio_wav_player::initialize()
{
    if (get_flag(state_initialized))
        return;

    if (!audio::device_manager.reg(this, device_index))
        throw audio::player_failed_to_register();

    global_error = Pa_OpenStream( &main_stream,
                                  NULL,
                                  &output_stream_params,
                                  sample_rate,
                                  0,
                                  paNoFlag,
                                  pa_stream_callback,
                                  (void *) this);

    if (global_error == paNoError)
        state_flags |= state_initialized;
}

void portaudio_wav_player::begin()
{
    if (!get_flag(state_initialized))
        throw audio::wav_player_not_initialized();

    if (get_flag(state_playing))
        return;

    global_error = Pa_StartStream(main_stream);
    if (global_error != paNoError)
        throw audio::playback_could_not_start();
    else
        state_flags |= state_playing;
}

void portaudio_wav_player::play()
{
    if (!get_flag(state_playing) && !get_flag(state_paused))
        return;

    global_error = Pa_StartStream(main_stream);

    if (global_error != paNoError)
        throw audio::playback_could_not_start();
    else
        state_flags &= ~state_paused;
}

void portaudio_wav_player::pause()
{
    if (!get_flag(state_playing) || get_flag(state_paused))
        return;

    global_error = Pa_StopStream(main_stream);

    if (global_error != paNoError) {
        throw audio::playback_could_not_stop();
    } else {
        state_flags |= state_paused;
    }
}

void portaudio_wav_player::toggle_pause()
{
    if (!get_flag(state_playing))
        return;

    if (get_flag(state_paused))
        play();
    else
        pause();
}

void portaudio_wav_player::stop()
{
    if (get_flag(state_stopped))
        return;

    global_error = Pa_StopStream(main_stream);

    if (global_error != paNoError) {
        throw audio::playback_could_not_stop();
    } else {
        read_stream.rewind();
        state_flags |= state_stopped;
    }
}

void portaudio_wav_player::seek(int bytes)
{
    pause();
    read_stream.seek(bytes);
    play();
}

bool portaudio_wav_player::get_playing() const
{
    return get_flag(state_playing);
}

bool portaudio_wav_player::get_paused() const
{
    return get_flag(state_playing);
}

bool portaudio_wav_player::get_flag(int mask) const
{
    return (state_flags & mask);
}

long portaudio_wav_player::fill_buffer(char **buffer,
                                       size_t frame_count)
{
    size_t bytes = frame_count * channel_count * 2;
    frames_last_read = read_stream.fill_buffer(buffer, bytes);

    return frames_last_read;
}

bool audio::initialize()
{
    return device_manager.initialize_portaudio();
}

bool audio::terminate()
{
    return device_manager.terminate_portaudio();
}

extern "C" play_wav *get_player_file(FILE *source, decoder *decoder,
                                     int playback_device_index)
{
    portaudio_wav_player *ret = new portaudio_wav_player(source, 2, decoder,
                                                         playback_device_index);
    ret->initialize();

    return ret;
}

extern "C" play_wav *get_player_memory(shared_memory source, decoder *decoder,
                                       int playback_device_index)
{
    portaudio_wav_player *ret = new portaudio_wav_player(source, 2, decoder,
                                                         playback_device_index);
    ret->initialize();

    return ret;
}
