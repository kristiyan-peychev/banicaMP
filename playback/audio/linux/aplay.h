#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <locale.h>
#include <alsa/asoundlib.h>
#include <assert.h>
#include <termios.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <endian.h>
#include <sys/ipc.h>
#include "aconfig.h"
#include "gettext.h"
#include "formats.h"
#include "version.h"

#include <fftw3.h> // fast fourier transform
#include <exception>
#include <functional>

#include "../../../../memory/memory.h"

#define DFT_BUFFER_SIZE 0x400

#ifdef SND_CHMAP_API_VERSION
#define CONFIG_SUPPORT_CHMAP    1
#endif

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

#ifndef le16toh
#include <asm/byteorder.h>
#define le16toh(x) __le16_to_cpu(x)
#define be16toh(x) __be16_to_cpu(x)
#define le32toh(x) __le32_to_cpu(x)
#define be32toh(x) __be32_to_cpu(x)
#endif

#define DEFAULT_FORMAT      SND_PCM_FORMAT_U8
#define DEFAULT_SPEED       8000

#define FORMAT_DEFAULT      -1
#define FORMAT_RAW      0
#define FORMAT_VOC      1
#define FORMAT_WAVE     2
#define FORMAT_AU       3

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...) do {\
    fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    putc('\n', stderr); \
} while (0)
#else
#define error(args...) do {\
    fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
    fprintf(stderr, ##args); \
    putc('\n', stderr); \
} while (0)
#endif

#define check_wavefile_space(buffer, len, blimit) \
    if (len > blimit) { \
        blimit = len; \
        if ((buffer = realloc(buffer, blimit)) == NULL) { \
            error(_("not enough memory"));        \
            prg_exit(EXIT_FAILURE);  \
        } \
    }

#ifndef timersub
#define timersub(a, b, result) \
do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) { \
        --(result)->tv_sec; \
        (result)->tv_usec += 1000000; \
    } \
} while (0)
#endif

#ifndef timermsub
#define timermsub(a, b, result) \
do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec; \
    if ((result)->tv_nsec < 0) { \
        --(result)->tv_sec; \
        (result)->tv_nsec += 1000000000L; \
    } \
} while (0)
#endif

#define check_wavefile_space(buffer, len, blimit) \
    if (len > blimit) { \
        blimit = len; \
        if ((buffer = (u_char*) realloc(buffer, blimit)) == NULL) { \
            throw audio::bad_alloc(); \
        } \
    }

enum {
    VUMETER_NONE,
    VUMETER_MONO,
    VUMETER_STEREO
};

enum {
    INITFLAG_DEFAULT    = 0x1,
    INITFLAG_FILENAME   = 0x2,
    INITFLAG_FILE       = 0x4
};

/**
 * class aplay
 * TODO
 * throws all exceptions from the audio namespace and may throw at any time
 */

class aplay {
public:
    snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle,
            void *buffer, snd_pcm_uframes_t size);
    snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle,
            const void *buffer, snd_pcm_uframes_t size);
    snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle,
            void **bufs, snd_pcm_uframes_t size);
    snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle,
            void **bufs, snd_pcm_uframes_t size);
private:
    fftw_complex       *dft_in;
    fftw_complex       *dft_out;
    fftw_plan           dft_plan;
private:
    int                 paused;
    char               *command;
    snd_pcm_t          *handle;

    struct {
        snd_pcm_format_t        format;
        unsigned int            channels;
        unsigned int            rate;
    }                   hwparams,
        rhwparams;

    int                 timelimit;
    int                 file_type;
    int                 open_mode;
    snd_pcm_stream_t    stream;
    int                 interleaved;
    int                 in_aborting;
    u_char             *audiobuf;
    snd_pcm_uframes_t   chunk_size;
    unsigned            period_time;
    unsigned            buffer_time;
    snd_pcm_uframes_t   period_frames;
    snd_pcm_uframes_t   buffer_frames;
    int                 avail_min;
    int                 start_delay;
    int                 stop_delay;
    int                 monotonic;
    int                 can_pause;
    int                 fatal_errors;
    int                 vumeter;
    int                 buffer_pos;
    size_t              bits_per_sample;
    size_t              bits_per_frame;
    size_t              chunk_bytes;
    int                 test_position;
    int                 test_coef;
    int                 test_nowait;
    snd_output_t       *log;
    int                 use_strftime;
    int                 dump_hw_params;
    int                 fd;
    off64_t             pbrec_count;
    off64_t             fdcount;
    int                 vocmajor;
    int                 vocminor;
    snd_pcm_chmap_t    *channel_map; /* chmap to override */
    unsigned int       *hw_map; /* chmap to follow */
private:
    std::function<size_t(void*, void *, size_t)> m_safe_read;
    memory_ref          sound_memory;
    FILE               *sound_file;
private:
    void plan_dft(void);
    void apply_filters_dft(void);
public:
    ~aplay(void);
    aplay(const char *pcm_name = "default");
    aplay(const aplay&);
public: // ex-main function, call this after constructed
    void init(const char *filename);
    void init(FILE *file);
    void init(memory_ref &);
    void init();
public:
    void seek(long miliseconds);
    void toggle_pause(void);

public:
    void show_available_sample_formats(snd_pcm_hw_params_t* params); // will print
private:
    void prg_exit(int code);
    ssize_t test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize);
    ssize_t test_wavefile(int fd, u_char *_buffer, size_t size);
    int setup_chmap(void);
    void set_params(void);
    void xrun(void); // I/O error handler
    void compute_max_peak(u_char *data, size_t count);
    void do_test_position(void);

    u_char *remap_data(u_char *data, size_t count);
    u_char **remap_datav(u_char **data, size_t count);

    ssize_t pcm_write(u_char *data, size_t count); // most important method

    void init_raw_data(void) { hwparams = rhwparams; }
    off64_t calc_count(void);

    void playback_go(int fd, size_t loaded, off64_t count, int rtype);
    void playback();
};

namespace audio {
    class exception : public std::exception {
    };

    class bad_alloc : public exception {
    public:
        const char *what() {
            return "Allocation failed";
        }
    };

    class bad_read : public exception {
    public:
        const char *what() {
            return "Unable to read the song file";
        }
    };

    class connection_failed : public exception {
        const char *what() {
            return "Connection to the audio output device failed";
        }
    };

    class info_retrieve_failed : public exception {
    public:
        const char *what() {
            return "Retrieval of the audio device information failed";
        }
    };

    class invalid_wave : public exception {
    public:
        const char *what() {
            return "The wave file supplied was incorrect";
        }
    };

    class wave_parse_error : public exception {
    public:
        const char *what() {
            return "Error parsing the wave file";
        }
    };
    class broken_pcm_confg : public exception {
    public:
        const char *what() {
            return "Broken configuration for this PCM: no configurations available";
        }
    };

    class access_type_not_available : public exception {
    public:
        const char *what() {
            return "PCM: Access type not available";
        }
    };

    class sample_format_not_available : public exception {
    public:
        const char *what() {
            return "PCM: Sample format not available";
        }
    };

    class channel_count_not_available : public exception {
    public:
        const char *what() {
            return "PCM: Channel count not available";
        }
    };

    class unable_to_install_hw_params : public exception {
    public:
        const char *what() {
            return "PCM: Could not install hw params";
        }
    };

    class unable_to_install_sw_params : public exception {
    public:
        const char *what() {
            return "PCM: Could not install sw params";
        }
    };

    class invalid_period : public exception {
    public:
        const char *what() {
            return "Can't use period equal to buffer size";
        }
    };

    class chmap_setup_failed : public exception {
    public:
        const char *what() {
            return "Could not setup chmap";
        }
    };
}
