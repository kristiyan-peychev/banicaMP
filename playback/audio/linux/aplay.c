/*
 *  aplay.c - plays and records
 *
 *      CREATIVE LABS CHANNEL-files
 *      Microsoft WAVE-files
 *      SPARC AUDIO .AU-files
 *      Raw Data
 *
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 *  Based on vplay program by Michael Beck
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#define _GNU_SOURCE
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

/* global data */

/* v custom */

#include <fftw3.h>

#define PARENT_PIPE_NAME "banica_player_chld_ctl_fifo"
#define SIZE 0x400

static int parent_pipe;
static _Bool filters_avail = 0;
static fftw_complex *dft_in;
static fftw_complex *dft_out;
static fftw_plan dft_plan;

/* ^ custom */

static snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle, const void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);

enum {
    VUMETER_NONE,
    VUMETER_MONO,
    VUMETER_STEREO
};

static int paused = 0;
static char *command;
static snd_pcm_t *handle;
static struct {
    snd_pcm_format_t format;
    unsigned int channels;
    unsigned int rate;
} hwparams, rhwparams;
static int timelimit = 0;
static int quiet_mode = 0;
static int file_type = FORMAT_DEFAULT;
static int open_mode = 0;
static snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
static int mmap_flag = 0;
static int interleaved = 1;
static int nonblock = 0;
static int in_aborting = 0;
static u_char *audiobuf = NULL;
static snd_pcm_uframes_t chunk_size = 0;
static unsigned period_time = 0;
static unsigned buffer_time = 0;
static snd_pcm_uframes_t period_frames = 0;
static snd_pcm_uframes_t buffer_frames = 0;
static int avail_min = -1;
static int start_delay = 0;
static int stop_delay = 0;
static int monotonic = 0;
static int can_pause = 0;
static int fatal_errors = 0;
static int verbose = 0;
static int vumeter = VUMETER_NONE;
static int buffer_pos = 0;
static size_t bits_per_sample, bits_per_frame;
static size_t chunk_bytes;
static int test_position = 0;
static int test_coef = 8;
static int test_nowait = 0;
static snd_output_t *log;
static long long max_file_size = 0;
static int max_file_time = 0;
static int use_strftime = 0;
static volatile int recycle_capture_file = 0;
static long term_c_lflag = -1;
static int dump_hw_params = 0;

static int fd = -1;
static off64_t pbrec_count = LLONG_MAX, fdcount;
static int vocmajor, vocminor;

static char *pidfile_name = NULL;
FILE *pidf = NULL;
static int pidfile_written = 0;

#ifdef CONFIG_SUPPORT_CHMAP
static snd_pcm_chmap_t *channel_map = NULL; /* chmap to override */
static unsigned int *hw_map = NULL; /* chmap to follow */
#endif

/* needed prototypes */


static void playback(char *filename);
static void playbackv(char **filenames, unsigned int count);

static void begin_wave(int fd, size_t count);
static void end_wave(int fd);

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

/*
 *  Subroutine to clean up before exit.
 */
static void prg_exit(int code) 
{
    close(parent_pipe);
    if (handle)
        snd_pcm_close(handle);
    if (pidfile_written)
        remove (pidfile_name);
    exit(code);
}

static void signal_handler(int sig)
{ /* TODO: remove/edit this? */
    close(parent_pipe);
    if (handle)
        snd_pcm_abort(handle);
    prg_exit(EXIT_FAILURE);
}

/* custom */

static inline void plan(void)
{
    dft_in = fftw_malloc(sizeof(*dft_in) * SIZE);
    dft_out = fftw_malloc(sizeof(*dft_out) * SIZE);
    dft_plan = fftw_plan_dft_1d(SIZE, dft_in, dft_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

static inline void apply_filters(void)
{
    /* TODO */
    fftw_execute(dft_plan);
}

static inline void do_apply_filters(u_char *data, size_t count)
{
    int i, j = 0;
    for (i = 0; j < count; i++)
        /*dft_in[i][0] = (double) ((data[j++] << 8) | data[j++]);*/
        data[j++] *= 0.6;

    /*apply_filters();*/
}

static void toggle_pause(int signum)
{
    if (!paused) {
        if (snd_pcm_pause(handle, 1) < 0) {
            fprintf(stderr, "Failed to pause\n");
            return;
        }
        paused = 1;
    } else {
      snd_pcm_pause(handle, 0);
        paused = 0;
    }
}

static void seek(int signum)
{
    int reed, sedem;
    if (read(parent_pipe, &reed, sizeof(reed)) < 0)
        return;

    toggle_pause(0);
    if (sedem = fseek(stdin, reed, SEEK_CUR))
        fprintf(stderr, "Error recieving seek value\n");
    toggle_pause(0);
}

/* /custom */

int main(int argc, char *argv[])
{
    int option_index;
    static const char short_options[] = "hnlLD:qt:c:f:r:d:MNF:A:R:T:B:vV:IPCi"
#ifdef CONFIG_SUPPORT_CHMAP
        "m:"
#endif
        ;
    struct sigaction sa;
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = toggle_pause;
    sigfillset(&sa.sa_mask);

    struct sigaction sb;
    sb.sa_flags = SA_RESTART;
    sb.sa_handler = seek;
    sigfillset(&sb.sa_mask);

    /*struct sigaction stest;*/
    /*stest.sa_flags = SA_RESTART;*/
    /*stest.sa_handler = vol_test;*/
    /*sigfillset(&stest.sa_mask);*/

    signal(SIGTERM, signal_handler);

    if (sigaction(SIGUSR1, &sa, NULL) == -1 || 
            /*sigaction(SIGABRT, &stest, NULL) == -1 || */
            sigaction(SIGINT, &sb, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    mknod(PARENT_PIPE_NAME, S_IFIFO | 0666, 0);
    parent_pipe = open(PARENT_PIPE_NAME, S_IFIFO | 0666, 0);
    plan();
    static const struct option long_options[] = {
        {"help", 0, 0, 'h'},
        {"list-devnames", 0, 0, 'n'},
        {"list-devices", 0, 0, 'l'},
        {"list-pcms", 0, 0, 'L'},
        {"device", 1, 0, 'D'},
        {"quiet", 0, 0, 'q'},
        {"file-type", 1, 0, 't'},
        {"channels", 1, 0, 'c'},
        {"format", 1, 0, 'f'},
        {"rate", 1, 0, 'r'},
        {"duration", 1, 0 ,'d'},
        {"mmap", 0, 0, 'M'},
        {"nonblock", 0, 0, 'N'},
        {"period-time", 1, 0, 'F'},
        {"avail-min", 1, 0, 'A'},
        {"start-delay", 1, 0, 'R'},
        {"stop-delay", 1, 0, 'T'},
        {"buffer-time", 1, 0, 'B'},
        {"verbose", 0, 0, 'v'},
        {"vumeter", 1, 0, 'V'},
        {"separate-channels", 0, 0, 'I'},
        {"playback", 0, 0, 'P'},
        {"capture", 0, 0, 'C'},
/*#ifdef CONFIG_SUPPORT_CHMAP*/
        /*{"chmap", 1, 0, 'm'},*/
/*#endif*/
        {0, 0, 0, 0}
    };
    char *pcm_name = "default";
    int tmp, err, c;
    snd_pcm_info_t *info;
    FILE *direction;

#ifdef ENABLE_NLS
    setlocale(LC_ALL, "");
    textdomain(PACKAGE);
#endif

    snd_pcm_info_alloca(&info);

    err = snd_output_stdio_attach(&log, stderr, 0);
    assert(err >= 0);

    file_type = FORMAT_DEFAULT;
    stream = SND_PCM_STREAM_PLAYBACK;
    command = "aplay";
    direction = stdin;

    chunk_size = -1;
    rhwparams.format = DEFAULT_FORMAT;
    rhwparams.rate = DEFAULT_SPEED;
    rhwparams.channels = 1;

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (c) {
        case 'D':
            pcm_name = optarg;
            break;
        case 'q':
            quiet_mode = 1;
            break;
        case 't':
            if (strcasecmp(optarg, "raw") == 0)
                file_type = FORMAT_RAW;
            else if (strcasecmp(optarg, "voc") == 0)
                file_type = FORMAT_VOC;
            else if (strcasecmp(optarg, "wav") == 0)
                file_type = FORMAT_WAVE;
            else if (strcasecmp(optarg, "au") == 0 || strcasecmp(optarg, "sparc") == 0)
                file_type = FORMAT_AU;
            else {
                error(_("unrecognized file format %s"), optarg);
                return 1;
            }
            break;
        case 'c':
            rhwparams.channels = strtol(optarg, NULL, 0);
            if (rhwparams.channels < 1 || rhwparams.channels > 256) {
                error(_("value %i for channels is invalid"), rhwparams.channels);
                return 1;
            }
            break;
        case 'f':
            if (strcasecmp(optarg, "cd") == 0 || strcasecmp(optarg, "cdr") == 0) {
                if (strcasecmp(optarg, "cdr") == 0)
                    rhwparams.format = SND_PCM_FORMAT_S16_BE;
                else
                    rhwparams.format = file_type == FORMAT_AU ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_S16_LE;
                rhwparams.rate = 44100;
                rhwparams.channels = 2;
            } else if (strcasecmp(optarg, "dat") == 0) {
                rhwparams.format = file_type == FORMAT_AU ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_S16_LE;
                rhwparams.rate = 48000;
                rhwparams.channels = 2;
            } else {
                rhwparams.format = snd_pcm_format_value(optarg);
                if (rhwparams.format == SND_PCM_FORMAT_UNKNOWN) {
                    error(_("wrong extended format '%s'"), optarg);
                    prg_exit(EXIT_FAILURE);
                }
            }
            break;
        case 'r':
            tmp = strtol(optarg, NULL, 0);
            if (tmp < 300)
                tmp *= 1000;
            rhwparams.rate = tmp;
            if (tmp < 2000 || tmp > 192000) {
                error(_("bad speed value %i"), tmp);
                return 1;
            }
            break;
        case 'd':
            timelimit = strtol(optarg, NULL, 0);
            break;
        case 'N':
            nonblock = 1;
            open_mode |= SND_PCM_NONBLOCK;
            break;
        case 'F':
            period_time = strtol(optarg, NULL, 0);
            break;
        case 'B':
            buffer_time = strtol(optarg, NULL, 0);
            break;
        case 'A':
            avail_min = strtol(optarg, NULL, 0);
            break;
        case 'R':
            start_delay = strtol(optarg, NULL, 0);
            break;
        case 'T':
            stop_delay = strtol(optarg, NULL, 0);
            break;
        case 'v':
            verbose++;
            if (verbose > 1 && !vumeter)
                vumeter = VUMETER_MONO;
            break;
        case 'V':
            if (*optarg == 's')
                vumeter = VUMETER_STEREO;
            else if (*optarg == 'm')
                vumeter = VUMETER_MONO;
            else
                vumeter = VUMETER_NONE;
            break;
        case 'M':
            mmap_flag = 1;
            break;
        case 'I':
            interleaved = 0;
            break;
        case 'P':
            stream = SND_PCM_STREAM_PLAYBACK;
            command = "aplay";
            break;
        case 'C':
            stream = SND_PCM_STREAM_CAPTURE;
            command = "arecord";
            start_delay = 1;
            if (file_type == FORMAT_DEFAULT)
                file_type = FORMAT_WAVE;
            break;
#ifdef CONFIG_SUPPORT_CHMAP
        case 'm':
            channel_map = snd_pcm_chmap_parse_string(optarg);
            if (!channel_map) {
                fprintf(stderr, _("Unable to parse channel map string: %s\n"), optarg);
                return 1;
            }
            break;
#endif
        default:
            fprintf(stderr, _("Try `%s --help' for more information.\n"), command);
            return 1;
        }
    }

    err = snd_pcm_open(&handle, pcm_name, stream, open_mode);
    if (err < 0) {
        error(_("audio open error: %s"), snd_strerror(err));
        return 1;
    }

    if ((err = snd_pcm_info(handle, info)) < 0) {
        error(_("info error: %s"), snd_strerror(err));
        return 1;
    }

    if (nonblock) {
        err = snd_pcm_nonblock(handle, 1);
        if (err < 0) {
            error(_("nonblock setting error: %s"), snd_strerror(err));
            return 1;
        }
    }

    chunk_size = 1024;
    hwparams = rhwparams;

    audiobuf = (u_char *)malloc(1024);
    if (audiobuf == NULL) {
        error(_("not enough memory"));
        return 1;
    }

    if (mmap_flag) {
        writei_func = snd_pcm_mmap_writei;
        readi_func = snd_pcm_mmap_readi;
        writen_func = snd_pcm_mmap_writen;
        readn_func = snd_pcm_mmap_readn;
    } else {
        writei_func = snd_pcm_writei;
        readi_func = snd_pcm_readi;
        writen_func = snd_pcm_writen;
        readn_func = snd_pcm_readn;
    }

    if (pidfile_name) {
        errno = 0;
        pidf = fopen (pidfile_name, "w");
        if (pidf) {
            (void)fprintf (pidf, "%d\n", getpid());
            fclose(pidf);
            pidfile_written = 1;
        } else {
            error(_("Cannot create process ID file %s: %s"), 
                pidfile_name, strerror (errno));
            return 1;
        }
    }

    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGINT, &sb, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if (interleaved) {
        if (optind > argc - 1) {
            /*if (stream == SND_PCM_STREAM_PLAYBACK)*/
                playback(NULL);
            /*else*/
                /*capture(NULL);*/
        } else {
            while (optind <= argc - 1) {
                /*if (stream == SND_PCM_STREAM_PLAYBACK)*/
                    playback(argv[optind++]); /* WTH? */
                /*else*/
                    /*capture(argv[optind++]);*/
            }
        }
    } else {
        /*if (stream == SND_PCM_STREAM_PLAYBACK)*/
            playbackv(&argv[optind], argc - optind);
        /*else*/
            /*capturev(&argv[optind], argc - optind);*/
    }
    snd_pcm_close(handle);
    handle = NULL;
    free(audiobuf);
      __end:
    snd_output_close(log);
    snd_config_update_free_global();
    prg_exit(EXIT_SUCCESS);
    /* avoid warning */
    return EXIT_SUCCESS;
}

/*
 * Safe read (for pipes)
 */
 
static ssize_t safe_read(int fd, void *buf, size_t count)
{
    ssize_t result = 0, res;

    while (count > 0 && !in_aborting) {
        if ((res = read(fd, buf, count)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}

/*
 * helper for test_wavefile
 */

static size_t test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line)
{
    if (*size >= reqsize)
        return *size;
    if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
        error(_("read error (called from line %i)"), line);
        prg_exit(EXIT_FAILURE);
    }
    return *size = reqsize;
}

#define check_wavefile_space(buffer, len, blimit) \
    if (len > blimit) { \
        blimit = len; \
        if ((buffer = realloc(buffer, blimit)) == NULL) { \
            error(_("not enough memory"));        \
            prg_exit(EXIT_FAILURE);  \
        } \
    }

/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 *                            == 0 if not
 * Value returned is bytes to be discarded.
 */
static ssize_t test_wavefile(int fd, u_char *_buffer, size_t size)
{
    WaveHeader *h = (WaveHeader *)_buffer;
    u_char *buffer = NULL;
    size_t blimit = 0;
    WaveFmtBody *f;
    WaveChunkHeader *c;
    u_int type, len;
    unsigned short format, channels;
    int big_endian, native_format;

    if (size < sizeof(WaveHeader))
        return -1;
    if (h->magic == WAV_RIFF)
        big_endian = 0;
    else if (h->magic == WAV_RIFX)
        big_endian = 1;
    if (h->type != WAV_WAVE)
        return -1;

    if (size > sizeof(WaveHeader)) {
        check_wavefile_space(buffer, size - sizeof(WaveHeader), blimit);
        memcpy(buffer, _buffer + sizeof(WaveHeader), size - sizeof(WaveHeader));
    }
    size -= sizeof(WaveHeader);
    while (1) {
        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = TO_CPU_INT(c->length, big_endian);
        len += len % 2;
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_FMT)
            break;
        check_wavefile_space(buffer, len, blimit);
        test_wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    if (len < sizeof(WaveFmtBody)) {
        error(_("unknown length of 'fmt ' chunk (read %u, should be %u at least)"),
              len, (u_int)sizeof(WaveFmtBody));
        prg_exit(EXIT_FAILURE);
    }
    check_wavefile_space(buffer, len, blimit);
    test_wavefile_read(fd, buffer, &size, len, __LINE__);
    f = (WaveFmtBody*) buffer;
    format = TO_CPU_SHORT(f->format, big_endian);
    if (format == WAV_FMT_EXTENSIBLE) {
        WaveFmtExtensibleBody *fe = (WaveFmtExtensibleBody*)buffer;
        if (len < sizeof(WaveFmtExtensibleBody)) {
            error(_("unknown length of extensible 'fmt ' chunk (read %u, should be %u at least)"),
                    len, (u_int)sizeof(WaveFmtExtensibleBody));
            prg_exit(EXIT_FAILURE);
        }
        if (memcmp(fe->guid_tag, WAV_GUID_TAG, 14) != 0) {
            error(_("wrong format tag in extensible 'fmt ' chunk"));
            prg_exit(EXIT_FAILURE);
        }
        format = TO_CPU_SHORT(fe->guid_format, big_endian);
    }
    if (format != WAV_FMT_PCM &&
        format != WAV_FMT_IEEE_FLOAT) {
                error(_("can't elay WAVE-file format 0x%04x which is not PCM or FLOAT encoded"), format);
        prg_exit(EXIT_FAILURE);
    }
    channels = TO_CPU_SHORT(f->channels, big_endian);
    if (channels < 1) {
        error(_("can't play WAVE-files with %d tracks"), channels);
        prg_exit(EXIT_FAILURE);
    }
    hwparams.channels = channels;
    switch (TO_CPU_SHORT(f->bit_p_spl, big_endian)) {
    case 8:
        if (hwparams.format != DEFAULT_FORMAT &&
            hwparams.format != SND_PCM_FORMAT_U8)
            fprintf(stderr, _("Warning: format is changed to U8\n"));
        hwparams.format = SND_PCM_FORMAT_U8;
        break;
    case 16:
        if (big_endian)
            native_format = SND_PCM_FORMAT_S16_BE;
        else
            native_format = SND_PCM_FORMAT_S16_LE;
        if (hwparams.format != DEFAULT_FORMAT &&
            hwparams.format != native_format)
            fprintf(stderr, _("Warning: format is changed to %s\n"),
                snd_pcm_format_name(native_format));
        hwparams.format = native_format;
        break;
    case 24:
        switch (TO_CPU_SHORT(f->byte_p_spl, big_endian) / hwparams.channels) {
        case 3:
            if (big_endian)
                native_format = SND_PCM_FORMAT_S24_3BE;
            else
                native_format = SND_PCM_FORMAT_S24_3LE;
            if (hwparams.format != DEFAULT_FORMAT &&
                hwparams.format != native_format)
                fprintf(stderr, _("Warning: format is changed to %s\n"),
                    snd_pcm_format_name(native_format));
            hwparams.format = native_format;
            break;
        case 4:
            if (big_endian)
                native_format = SND_PCM_FORMAT_S24_BE;
            else
                native_format = SND_PCM_FORMAT_S24_LE;
            if (hwparams.format != DEFAULT_FORMAT &&
                hwparams.format != native_format)
                fprintf(stderr, _("Warning: format is changed to %s\n"),
                    snd_pcm_format_name(native_format));
            hwparams.format = native_format;
            break;
        default:
            error(_(" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)"),
                  TO_CPU_SHORT(f->bit_p_spl, big_endian),
                  TO_CPU_SHORT(f->byte_p_spl, big_endian),
                  hwparams.channels);
            prg_exit(EXIT_FAILURE);
        }
        break;
    case 32:
        if (format == WAV_FMT_PCM) {
            if (big_endian)
                native_format = SND_PCM_FORMAT_S32_BE;
            else
                native_format = SND_PCM_FORMAT_S32_LE;
                        hwparams.format = native_format;
        } else if (format == WAV_FMT_IEEE_FLOAT) {
            if (big_endian)
                native_format = SND_PCM_FORMAT_FLOAT_BE;
            else
                native_format = SND_PCM_FORMAT_FLOAT_LE;
            hwparams.format = native_format;
        }
        break;
    default:
        error(_(" can't play WAVE-files with sample %d bits wide"),
              TO_CPU_SHORT(f->bit_p_spl, big_endian));
        prg_exit(EXIT_FAILURE);
    }
    hwparams.rate = TO_CPU_INT(f->sample_fq, big_endian);
    
    if (size > len)
        memmove(buffer, buffer + len, size - len);
    size -= len;
    
    while (1) {
        u_int type, len;

        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = TO_CPU_INT(c->length, big_endian);
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_DATA) {
            if (len < pbrec_count && len < 0x7ffffffe)
                pbrec_count = len;
            if (size > 0)
                memcpy(_buffer, buffer, size);
            free(buffer);
            return size;
        }
        len += len % 2;
        check_wavefile_space(buffer, len, blimit);
        test_wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    /* shouldn't be reached */
    return -1;
}

/*

 */

static int test_au(int fd, void *buffer)
{
    AuHeader *ap = buffer;

    if (ap->magic != AU_MAGIC)
        return -1;
    if (BE_INT(ap->hdr_size) > 128 || BE_INT(ap->hdr_size) < 24)
        return -1;
    pbrec_count = BE_INT(ap->data_size);
    switch (BE_INT(ap->encoding)) {
    case AU_FMT_ULAW:
        if (hwparams.format != DEFAULT_FORMAT &&
            hwparams.format != SND_PCM_FORMAT_MU_LAW)
            fprintf(stderr, _("Warning: format is changed to MU_LAW\n"));
        hwparams.format = SND_PCM_FORMAT_MU_LAW;
        break;
    case AU_FMT_LIN8:
        if (hwparams.format != DEFAULT_FORMAT &&
            hwparams.format != SND_PCM_FORMAT_U8)
            fprintf(stderr, _("Warning: format is changed to U8\n"));
        hwparams.format = SND_PCM_FORMAT_U8;
        break;
    case AU_FMT_LIN16:
        if (hwparams.format != DEFAULT_FORMAT &&
            hwparams.format != SND_PCM_FORMAT_S16_BE)
            fprintf(stderr, _("Warning: format is changed to S16_BE\n"));
        hwparams.format = SND_PCM_FORMAT_S16_BE;
        break;
    default:
        return -1;
    }
    hwparams.rate = BE_INT(ap->sample_rate);
    if (hwparams.rate < 2000 || hwparams.rate > 256000)
        return -1;
    hwparams.channels = BE_INT(ap->channels);
    if (hwparams.channels < 1 || hwparams.channels > 256)
        return -1;
    if ((size_t)safe_read(fd, buffer + sizeof(AuHeader), BE_INT(ap->hdr_size) - sizeof(AuHeader)) != BE_INT(ap->hdr_size) - sizeof(AuHeader)) {
        error(_("read error"));
        prg_exit(EXIT_FAILURE);
    }
    return 0;
}

static void show_available_sample_formats(snd_pcm_hw_params_t* params)
{
    snd_pcm_format_t format;

    fprintf(stderr, "Available formats:\n");
    for (format = 0; format <= SND_PCM_FORMAT_LAST; format++) {
        if (snd_pcm_hw_params_test_format(handle, params, format) == 0)
            fprintf(stderr, "- %s\n", snd_pcm_format_name(format));
    }
}

#ifdef CONFIG_SUPPORT_CHMAP
static int setup_chmap(void)
{
    snd_pcm_chmap_t *chmap = channel_map;
    char mapped[hwparams.channels];
    snd_pcm_chmap_t *hw_chmap;
    unsigned int ch, i;
    int err;

    if (!chmap)
        return 0;

    if (chmap->channels != hwparams.channels) {
        error(_("Channel numbers don't match between hw_params and channel map"));
        return -1;
    }
    err = snd_pcm_set_chmap(handle, chmap);
    if (!err)
        return 0;

    hw_chmap = snd_pcm_get_chmap(handle);
    if (!hw_chmap) {
        fprintf(stderr, _("Warning: unable to get channel map\n"));
        return 0;
    }

    if (hw_chmap->channels == chmap->channels &&
        !memcmp(hw_chmap, chmap, 4 * (chmap->channels + 1))) {
        /* maps are identical, so no need to convert */
        free(hw_chmap);
        return 0;
    }

    hw_map = calloc(hwparams.channels, sizeof(int));
    if (!hw_map) {
        error(_("not enough memory"));
        return -1;
    }

    memset(mapped, 0, sizeof(mapped));
    for (ch = 0; ch < hw_chmap->channels; ch++) {
        if (chmap->pos[ch] == hw_chmap->pos[ch]) {
            mapped[ch] = 1;
            hw_map[ch] = ch;
            continue;
        }
        for (i = 0; i < hw_chmap->channels; i++) {
            if (!mapped[i] && chmap->pos[ch] == hw_chmap->pos[i]) {
                mapped[i] = 1;
                hw_map[ch] = i;
                break;
            }
        }
        if (i >= hw_chmap->channels) {
            char buf[256];
            error(_("Channel %d doesn't match with hw_parmas"), ch);
            snd_pcm_chmap_print(hw_chmap, sizeof(buf), buf);
            fprintf(stderr, "hardware chmap = %s\n", buf);
            return -1;
        }
    }
    free(hw_chmap);
    return 0;
}
#else
#define setup_chmap()   0
#endif

static void set_params(void)
{
    snd_pcm_hw_params_t *params;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_uframes_t buffer_size;
    int err;
    size_t n;
    unsigned int rate;
    snd_pcm_uframes_t start_threshold, stop_threshold;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_sw_params_alloca(&swparams);
    err = snd_pcm_hw_params_any(handle, params);
    if (err < 0) {
        error(_("Broken configuration for this PCM: no configurations available"));
        prg_exit(EXIT_FAILURE);
    }
    if (dump_hw_params) {
        fprintf(stderr, _("HW Params of device \"%s\":\n"),
            snd_pcm_name(handle));
        fprintf(stderr, "--------------------\n");
        snd_pcm_hw_params_dump(params, log);
        fprintf(stderr, "--------------------\n");
    }
    if (mmap_flag) {
        snd_pcm_access_mask_t *mask = alloca(snd_pcm_access_mask_sizeof());
        snd_pcm_access_mask_none(mask);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_COMPLEX);
        err = snd_pcm_hw_params_set_access_mask(handle, params, mask);
    } else if (interleaved)
        err = snd_pcm_hw_params_set_access(handle, params,
                           SND_PCM_ACCESS_RW_INTERLEAVED);
    else
        err = snd_pcm_hw_params_set_access(handle, params,
                           SND_PCM_ACCESS_RW_NONINTERLEAVED);
    if (err < 0) {
        error(_("Access type not available"));
        prg_exit(EXIT_FAILURE);
    }
    err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
    if (err < 0) {
        error(_("Sample format non available"));
        show_available_sample_formats(params);
        prg_exit(EXIT_FAILURE);
    }
    err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
    if (err < 0) {
        error(_("Channels count non available"));
        prg_exit(EXIT_FAILURE);
    }

#if 0
    err = snd_pcm_hw_params_set_periods_min(handle, params, 2);
    assert(err >= 0);
#endif
    rate = hwparams.rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
    assert(err >= 0);
    if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate) {
        if (!quiet_mode) {
            char plugex[64];
            const char *pcmname = snd_pcm_name(handle);
            fprintf(stderr, _("Warning: rate is not accurate (requested = %iHz, got = %iHz)\n"), rate, hwparams.rate);
            if (! pcmname || strchr(snd_pcm_name(handle), ':'))
                *plugex = 0;
            else
                snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
                     snd_pcm_name(handle));
            fprintf(stderr, _("         please, try the plug plugin %s\n"),
                plugex);
        }
    }
    rate = hwparams.rate;
    if (buffer_time == 0 && buffer_frames == 0) {
        err = snd_pcm_hw_params_get_buffer_time_max(params,
                                &buffer_time, 0);
        assert(err >= 0);
        if (buffer_time > 500000)
            buffer_time = 500000;
    }
    if (period_time == 0 && period_frames == 0) {
        if (buffer_time > 0)
            period_time = buffer_time / 4;
        else
            period_frames = buffer_frames / 4;
    }
    if (period_time > 0)
        err = snd_pcm_hw_params_set_period_time_near(handle, params,
                                 &period_time, 0);
    else
        err = snd_pcm_hw_params_set_period_size_near(handle, params,
                                 &period_frames, 0);
    assert(err >= 0);
    if (buffer_time > 0) {
        err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
                                 &buffer_time, 0);
    } else {
        err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
                                 &buffer_frames);
    }
    assert(err >= 0);
    monotonic = snd_pcm_hw_params_is_monotonic(params);
    can_pause = snd_pcm_hw_params_can_pause(params);
    err = snd_pcm_hw_params(handle, params);
    if (err < 0) {
        error(_("Unable to install hw params:"));
        snd_pcm_hw_params_dump(params, log);
        prg_exit(EXIT_FAILURE);
    }
    snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
    if (chunk_size == buffer_size) {
        error(_("Can't use period equal to buffer size (%lu == %lu)"),
              chunk_size, buffer_size);
        prg_exit(EXIT_FAILURE);
    }
    snd_pcm_sw_params_current(handle, swparams);
    if (avail_min < 0)
        n = chunk_size;
    else
        n = (double) rate * avail_min / 1000000;
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

    /* round up to closest transfer boundary */
    n = buffer_size;
    if (start_delay <= 0) {
        start_threshold = n + (double) rate * start_delay / 1000000;
    } else
        start_threshold = (double) rate * start_delay / 1000000;
    if (start_threshold < 1)
        start_threshold = 1;
    if (start_threshold > n)
        start_threshold = n;
    err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
    assert(err >= 0);
    if (stop_delay <= 0) 
        stop_threshold = buffer_size + (double) rate * stop_delay / 1000000;
    else
        stop_threshold = (double) rate * stop_delay / 1000000;
    err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
    assert(err >= 0);

    if (snd_pcm_sw_params(handle, swparams) < 0) {
        error(_("unable to install sw params:"));
        snd_pcm_sw_params_dump(swparams, log);
        prg_exit(EXIT_FAILURE);
    }

    if (setup_chmap())
        prg_exit(EXIT_FAILURE);

    if (verbose)
        snd_pcm_dump(handle, log);

    bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
    bits_per_frame = bits_per_sample * hwparams.channels;
    chunk_bytes = chunk_size * bits_per_frame / 8;
    audiobuf = realloc(audiobuf, chunk_bytes);
    if (audiobuf == NULL) {
        error(_("not enough memory"));
        prg_exit(EXIT_FAILURE);
    }
    /*fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);*/

    /* stereo VU-meter isn't always available... */
    if (vumeter == VUMETER_STEREO) {
        if (hwparams.channels != 2 || !interleaved || verbose > 2)
            vumeter = VUMETER_MONO;
    }

    /* show mmap buffer arragment */
    if (mmap_flag && verbose) {
        const snd_pcm_channel_area_t *areas;
        snd_pcm_uframes_t offset, size = chunk_size;
        int i;
        err = snd_pcm_mmap_begin(handle, &areas, &offset, &size);
        if (err < 0) {
            error(_("snd_pcm_mmap_begin problem: %s"), snd_strerror(err));
            prg_exit(EXIT_FAILURE);
        }
        for (i = 0; i < hwparams.channels; i++)
            fprintf(stderr, "mmap_area[%i] = %p,%u,%u (%u)\n", i, areas[i].addr, areas[i].first, areas[i].step, snd_pcm_format_physical_width(hwparams.format));
        /* not required, but for sure */
        snd_pcm_mmap_commit(handle, offset, 0);
    }

    buffer_frames = buffer_size;    /* for position test */
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

/* I/O error handler */
static void xrun(void)
{
    snd_pcm_status_t *status;
    int res;
    
    snd_pcm_status_alloca(&status);
    if ((res = snd_pcm_status(handle, status))<0) {
        error(_("status error: %s"), snd_strerror(res));
        prg_exit(EXIT_FAILURE);
    }
    if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
        if (fatal_errors) {
            error(_("fatal %s: %s"),
                    stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
                    snd_strerror(res));
            prg_exit(EXIT_FAILURE);
        }
        if (monotonic) {
#ifdef HAVE_CLOCK_GETTIME
            struct timespec now, diff, tstamp;
            clock_gettime(CLOCK_MONOTONIC, &now);
            snd_pcm_status_get_trigger_htstamp(status, &tstamp);
            timermsub(&now, &tstamp, &diff);
            fprintf(stderr, _("%s!!! (at least %.3f ms long)\n"),
                stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
                diff.tv_sec * 1000 + diff.tv_nsec / 1000000.0);
#else
            fprintf(stderr, "%s !!!\n", _("underrun"));
#endif
        } else {
            struct timeval now, diff, tstamp;
            gettimeofday(&now, 0);
            snd_pcm_status_get_trigger_tstamp(status, &tstamp);
            timersub(&now, &tstamp, &diff);
            fprintf(stderr, _("%s!!! (at least %.3f ms long)\n"),
                stream == SND_PCM_STREAM_PLAYBACK ? _("underrun") : _("overrun"),
                diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
        }
        if (verbose) {
            fprintf(stderr, _("Status:\n"));
            snd_pcm_status_dump(status, log);
        }
        if ((res = snd_pcm_prepare(handle))<0) {
            error(_("xrun: prepare error: %s"), snd_strerror(res));
            prg_exit(EXIT_FAILURE);
        }
        return;     /* ok, data should be accepted again */
    } if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
        if (verbose) {
            fprintf(stderr, _("Status(DRAINING):\n"));
            snd_pcm_status_dump(status, log);
        }
        if (stream == SND_PCM_STREAM_CAPTURE) {
            fprintf(stderr, _("capture stream format change? attempting recover...\n"));
            if ((res = snd_pcm_prepare(handle))<0) {
                error(_("xrun(DRAINING): prepare error: %s"), snd_strerror(res));
                prg_exit(EXIT_FAILURE);
            }
            return;
        }
    }
    if (verbose) {
        fprintf(stderr, _("Status(R/W):\n"));
        snd_pcm_status_dump(status, log);
    }
    error(_("read/write error, state = %s"), snd_pcm_state_name(snd_pcm_status_get_state(status)));
    prg_exit(EXIT_FAILURE);
}

/* I/O suspend handler */
static void suspend(void)
{
    int res;

    fprintf(stderr, "TEST: resume?\n");
    if (!quiet_mode)
        fprintf(stderr, _("Suspended. Trying resume. ")); fflush(stderr);
    while ((res = snd_pcm_resume(handle)) == -EAGAIN)
        sleep(1);   /* wait until suspend flag is released */
    if (res < 0) {
        if (!quiet_mode)
            fprintf(stderr, _("Failed. Restarting stream. ")); fflush(stderr);
        if ((res = snd_pcm_prepare(handle)) < 0) {
            error(_("suspend: prepare error: %s"), snd_strerror(res));
            prg_exit(EXIT_FAILURE);
        }
    }
    if (!quiet_mode)
        fprintf(stderr, _("Done.\n"));
}

static void print_vu_meter_mono(int perc, int maxperc)
{
    const int bar_length = 50;
    char line[80];
    int val;

    for (val = 0; val <= perc * bar_length / 100 && val < bar_length; val++)
        line[val] = '#';
    for (; val <= maxperc * bar_length / 100 && val < bar_length; val++)
        line[val] = ' ';
    line[val] = '+';
    for (++val; val <= bar_length; val++)
        line[val] = ' ';
    if (maxperc > 99)
        sprintf(line + val, "| MAX");
    else
        sprintf(line + val, "| %02i%%", maxperc);
    fputs(line, stderr);
    if (perc > 100)
        fprintf(stderr, _(" !clip  "));
}

static void print_vu_meter_stereo(int *perc, int *maxperc)
{
    const int bar_length = 35;
    char line[80];
    int c;

    memset(line, ' ', sizeof(line) - 1);
    line[bar_length + 3] = '|';

    for (c = 0; c < 2; c++) {
        int p = perc[c] * bar_length / 100;
        char tmp[4];
        if (p > bar_length)
            p = bar_length;
        if (c)
            memset(line + bar_length + 6 + 1, '#', p);
        else
            memset(line + bar_length - p - 1, '#', p);
        p = maxperc[c] * bar_length / 100;
        if (p > bar_length)
            p = bar_length;
        if (c)
            line[bar_length + 6 + 1 + p] = '+';
        else
            line[bar_length - p - 1] = '+';
        if (maxperc[c] > 99)
            sprintf(tmp, "MAX");
        else
            sprintf(tmp, "%02d%%", maxperc[c]);
        if (c)
            memcpy(line + bar_length + 3 + 1, tmp, 3);
        else
            memcpy(line + bar_length, tmp, 3);
    }
    line[bar_length * 2 + 6 + 2] = 0;
    fputs(line, stderr);
}

static void print_vu_meter(signed int *perc, signed int *maxperc)
{
    if (vumeter == VUMETER_STEREO)
        print_vu_meter_stereo(perc, maxperc);
    else
        print_vu_meter_mono(*perc, *maxperc);
}

/* peak handler */
static void compute_max_peak(u_char *data, size_t count)
{
    signed int val, max, perc[2], max_peak[2];
    static  int run = 0;
    size_t ocount = count;
    int format_little_endian = snd_pcm_format_little_endian(hwparams.format);   
    int ichans, c;

    if (vumeter == VUMETER_STEREO)
        ichans = 2;
    else
        ichans = 1;

    memset(max_peak, 0, sizeof(max_peak));
    switch (bits_per_sample) {
    case 8: {
        signed char *valp = (signed char *)data;
        signed char mask = snd_pcm_format_silence(hwparams.format);
        c = 0;
        while (count-- > 0) {
            val = *valp++ ^ mask;
            val = abs(val);
            if (max_peak[c] < val)
                max_peak[c] = val;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 16: {
        signed short *valp = (signed short *)data;
        signed short mask = snd_pcm_format_silence_16(hwparams.format);
        signed short sval;

        count /= 2;
        c = 0;
        while (count-- > 0) {
            if (format_little_endian)
                sval = le16toh(*valp);
            else
                sval = be16toh(*valp);
            sval = abs(sval) ^ mask;
            if (max_peak[c] < sval)
                max_peak[c] = sval;
            valp++;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 24: {
        unsigned char *valp = data;
        signed int mask = snd_pcm_format_silence_32(hwparams.format);

        count /= 3;
        c = 0;
        while (count-- > 0) {
            if (format_little_endian) {
                val = valp[0] | (valp[1]<<8) | (valp[2]<<16);
            } else {
                val = (valp[0]<<16) | (valp[1]<<8) | valp[2];
            }
            /* Correct signed bit in 32-bit value */
            if (val & (1<<(bits_per_sample-1))) {
                val |= 0xff<<24;    /* Negate upper bits too */
            }
            val = abs(val) ^ mask;
            if (max_peak[c] < val)
                max_peak[c] = val;
            valp += 3;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    case 32: {
        signed int *valp = (signed int *)data;
        signed int mask = snd_pcm_format_silence_32(hwparams.format);

        count /= 4;
        c = 0;
        while (count-- > 0) {
            if (format_little_endian)
                val = le32toh(*valp);
            else
                val = be32toh(*valp);
            val = abs(val) ^ mask;
            if (max_peak[c] < val)
                max_peak[c] = val;
            valp++;
            if (vumeter == VUMETER_STEREO)
                c = !c;
        }
        break;
    }
    default:
        if (run == 0) {
            fprintf(stderr, _("Unsupported bit size %d.\n"), (int)bits_per_sample);
            run = 1;
        }
        return;
    }
    max = 1 << (bits_per_sample-1);
    if (max <= 0)
        max = 0x7fffffff;

    for (c = 0; c < ichans; c++) {
        if (bits_per_sample > 16)
            perc[c] = max_peak[c] / (max / 100);
        else
            perc[c] = max_peak[c] * 100 / max;
    }

    if (interleaved && verbose <= 2) {
        static int maxperc[2];
        static time_t t=0;
        const time_t tt=time(NULL);
        if(tt>t) {
            t=tt;
            maxperc[0] = 0;
            maxperc[1] = 0;
        }
        for (c = 0; c < ichans; c++)
            if (perc[c] > maxperc[c])
                maxperc[c] = perc[c];

        putc('\r', stderr);
        print_vu_meter(perc, maxperc);
        fflush(stderr);
    }
    else if(verbose==3) {
        fprintf(stderr, _("Max peak (%li samples): 0x%08x "), (long)ocount, max_peak[0]);
        for (val = 0; val < 20; val++)
            if (val <= perc[0] / 5)
                putc('#', stderr);
            else
                putc(' ', stderr);
        fprintf(stderr, " %i%%\n", perc[0]);
        fflush(stderr);
    }
}

static void do_test_position(void)
{
    static long counter = 0;
    static time_t tmr = -1;
    time_t now;
    static float availsum, delaysum, samples;
    static snd_pcm_sframes_t maxavail, maxdelay;
    static snd_pcm_sframes_t minavail, mindelay;
    static snd_pcm_sframes_t badavail = 0, baddelay = 0;
    snd_pcm_sframes_t outofrange;
    snd_pcm_sframes_t avail, delay;
    int err;

    err = snd_pcm_avail_delay(handle, &avail, &delay);
    if (err < 0)
        return;
    outofrange = (test_coef * (snd_pcm_sframes_t)buffer_frames) / 2;
    if (avail > outofrange || avail < -outofrange ||
        delay > outofrange || delay < -outofrange) {
      badavail = avail; baddelay = delay;
      availsum = delaysum = samples = 0;
      maxavail = maxdelay = 0;
      minavail = mindelay = buffer_frames * 16;
      fprintf(stderr, _("Suspicious buffer position (%li total): "
        "avail = %li, delay = %li, buffer = %li\n"),
        ++counter, (long)avail, (long)delay, (long)buffer_frames);
    } else if (verbose) {
        time(&now);
        if (tmr == (time_t) -1) {
            tmr = now;
            availsum = delaysum = samples = 0;
            maxavail = maxdelay = 0;
            minavail = mindelay = buffer_frames * 16;
        }
        if (avail > maxavail)
            maxavail = avail;
        if (delay > maxdelay)
            maxdelay = delay;
        if (avail < minavail)
            minavail = avail;
        if (delay < mindelay)
            mindelay = delay;
        availsum += avail;
        delaysum += delay;
        samples++;
        if (avail != 0 && now != tmr) {
            fprintf(stderr, "BUFPOS: avg%li/%li "
                "min%li/%li max%li/%li (%li) (%li:%li/%li)\n",
                (long)(availsum / samples),
                (long)(delaysum / samples),
                (long)minavail, (long)mindelay,
                (long)maxavail, (long)maxdelay,
                (long)buffer_frames,
                counter, badavail, baddelay);
            tmr = now;
        }
    }
}

/*
 */
#ifdef CONFIG_SUPPORT_CHMAP
static u_char *remap_data(u_char *data, size_t count)
{
    static u_char *tmp, *src, *dst;
    static size_t tmp_size;
    size_t sample_bytes = bits_per_sample / 8;
    size_t step = bits_per_frame / 8;
    size_t chunk_bytes;
    unsigned int ch, i;

    if (!hw_map)
        return data;

    chunk_bytes = count * bits_per_frame / 8;
    if (tmp_size < chunk_bytes) {
        free(tmp);
        tmp = malloc(chunk_bytes);
        if (!tmp) {
            error(_("not enough memory"));
            exit(1);
        }
        tmp_size = count;
    }

    src = data;
    dst = tmp;
    for (i = 0; i < count; i++) {
        for (ch = 0; ch < hwparams.channels; ch++) {
            memcpy(dst, src + sample_bytes * hw_map[ch],
                   sample_bytes);
            dst += sample_bytes;
        }
        src += step;
    }
    return tmp;
}

static u_char **remap_datav(u_char **data, size_t count)
{
    static u_char **tmp;
    unsigned int ch;

    if (!hw_map)
        return data;

    if (!tmp) {
        tmp = malloc(sizeof(*tmp) * hwparams.channels);
        if (!tmp) {
            error(_("not enough memory"));
            exit(1);
        }
        for (ch = 0; ch < hwparams.channels; ch++)
            tmp[ch] = data[hw_map[ch]];
    }
    return tmp;
}
#else
#define remap_data(data, count)     (data)
#define remap_datav(data, count)    (data)
#endif

static ssize_t pcm_write(u_char *data, size_t count)
{
    ssize_t r;
    ssize_t result = 0;

    if (count < chunk_size) {
        snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
        count = chunk_size;
    }
    data = remap_data(data, count);
    while (count > 0 && !in_aborting) {
        if (test_position)
            do_test_position();
        r = writei_func(handle, data, count);
        if (test_position)
            do_test_position();
        if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
            if (!test_nowait)
                snd_pcm_wait(handle, 100);
        } else if (r == -EPIPE) {
            xrun();
        } else if (r == -ESTRPIPE) {
            suspend();
        } else if (r < 0) {
            error(_("write error: %s"), snd_strerror(r));
            prg_exit(EXIT_FAILURE);
        }
        if (r > 0) {
            if (vumeter)
                compute_max_peak(data, r * hwparams.channels);
            result += r;
            count -= r;
            data += r * bits_per_frame / 8;
        }
    }
    return result;
}

/* setting the globals for playing raw data */
static inline void init_raw_data(void)
{
    hwparams = rhwparams;
}

/* calculate the data count to read from/to dsp */
static off64_t calc_count(void)
{
    off64_t count;

    if (timelimit == 0) {
        count = pbrec_count;
    } else {
        count = snd_pcm_format_size(hwparams.format, hwparams.rate * hwparams.channels);
        count *= (off64_t)timelimit;
    }
    return count < pbrec_count ? count : pbrec_count;
}

/* playing raw data */
static void playback_go(int fd, size_t loaded, off64_t count, int rtype, char *name)
{
    int l, r;
    off64_t written = 0;
    off64_t c;

    set_params();

    while (loaded > chunk_bytes && written < count && !in_aborting) {
        if (filters_avail)
            do_apply_filters(audiobuf + written, chunk_size);

        if (pcm_write(audiobuf + written, chunk_size) <= 0)
            return;
        written += chunk_bytes;
        loaded -= chunk_bytes;
    }
    if (written > 0 && loaded > 0)
        memmove(audiobuf, audiobuf + written, loaded);

    l = loaded;
    while (written < count && !in_aborting) {
        do {
            c = count - written;
            if (c > chunk_bytes)
                c = chunk_bytes;
            c -= l;

            if (c == 0)
                break;
            r = safe_read(fd, audiobuf + l, c);
            if (r < 0) {
                perror(name);
                prg_exit(EXIT_FAILURE);
            }
            fdcount += r;
            if (r == 0)
                break;
            l += r;
        } while ((size_t)l < chunk_bytes);
        l = l * 8 / bits_per_frame;
        r = pcm_write(audiobuf, l);
        if (r != l)
            break;
        r = r * bits_per_frame / 8;
        written += r;
        l = 0;
    }
    snd_pcm_nonblock(handle, 0);
    snd_pcm_drain(handle);
    snd_pcm_nonblock(handle, nonblock);
}

static void playback(char *name)
{
    int ofs;
    size_t dta;
    ssize_t dtawave;

    pbrec_count = LLONG_MAX;
    fdcount = 0;
    fd = fileno(stdin);
    name = "stdin";
    /* read bytes for WAVE-header */
    if ((dtawave = test_wavefile(fd, audiobuf, dta)) >= 0) {
        pbrec_count = calc_count();
        playback_go(fd, dtawave, pbrec_count, FORMAT_WAVE, name);
    } else {
        /* should be raw data */
        init_raw_data();
        pbrec_count = calc_count();
        playback_go(fd, dta, pbrec_count, FORMAT_RAW, name);
    }
      __end:
    if (fd != 0)
        close(fd);
}

static void playbackv(char **names, unsigned int count)
{
    int ret = 0;
    unsigned int channel;
    unsigned int channels = rhwparams.channels;
    int alloced = 0;
    int fds[channels];
    for (channel = 0; channel < channels; ++channel)
        fds[channel] = -1;

    if (count == 1 && channels > 1) {
        size_t len = strlen(names[0]);
        char format[1024];
        memcpy(format, names[0], len);
        strcpy(format + len, ".%d");
        len += 4;
        names = malloc(sizeof(*names) * channels);
        for (channel = 0; channel < channels; ++channel) {
            names[channel] = malloc(len);
            sprintf(names[channel], format, channel);
        }
        alloced = 1;
    } else if (count != channels) {
        prg_exit(EXIT_FAILURE);
    }

    for (channel = 0; channel < channels; ++channel) {
        fds[channel] = open(names[channel], O_RDONLY, 0);
        if (fds[channel] < 0) {
            perror(names[channel]);
            ret = EXIT_FAILURE;
            goto __end;
        }
    }
    __end:
    for (channel = 0; channel < channels; ++channel) {
        if (fds[channel] >= 0)
            close(fds[channel]);
        if (alloced)
            free(names[channel]);
    }
    if (alloced)
        free(names);
    if (ret)
        prg_exit(ret);
}
