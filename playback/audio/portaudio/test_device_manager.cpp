/* g++ device_manager.cpp test_device_manager.cpp audio.cpp stream.cpp -std=c++11 -Wall -lportaudio -g /home/kawaguchi/banicaMP/memory/build/libmem.a */
#include "device_manager.h"
#include "audio.h"
#include <cassert>

int main(int argc, const char *argv[]) {
    portaudio_enumeration_manager enumerator;
    printf("Initializing portaudio\n");
    audio::initialize();
    printf("Portaudio initialized\n");

    assert(enumerator.enumerate());
    const output_device *itr = enumerator.get_enumeration();
    while (itr) {
        printf("Enumerated device: %s, channel count %d, sample rate %lf\n", itr->get_name(), itr->get_max_channels(), itr->get_default_sample_rate());
        itr = itr->get_next();
    }

    FILE *source = fopen("/home/kawaguchi/Windfall_decoded.wav", "rw+");
    if (source == NULL) {
        exit(1);
    }
    portaudio_wav_player player(source, 2);
    player.initialize();
    player.begin();
    Pa_Sleep(50*1000);

    printf("Terminating portaudio\n");
    audio::terminate();
    printf("Portaudio terminated\n");
return 0;
}

