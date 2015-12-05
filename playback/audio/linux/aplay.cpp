#include "aplay.h"

static ssize_t safe_read_file(void *fdp, void *buf, size_t count)
{
    int fd = (int) (long) fdp; // cheat
    ssize_t result = 0, res;

    while (count > 0) {
        if ((res = read(fd, buf, count)) == 0)
            break;
        if (res < 0)
            return (result > 0 ? result : res);
        count -= res;
        result += res;
        buf = (char *) buf + res;
    }
    return result;
}

static ssize_t safe_read_memory(void *memptr, void *buf, size_t count)
{
    memory_ref memory = *((memory_ref *) memptr);
    char *data = memory.read(count);
    return (ssize_t) count;
}

void aplay::plan_dft(void)
{
    //dft_in   = (fftw_complex *) fftw_malloc(sizeof(*dft_in) * DFT_BUFFER_SIZE);
    //dft_out  = (fftw_complex *) fftw_malloc(sizeof(*dft_out) * DFT_BUFFER_SIZE);
    //dft_plan = fftw_plan_dft_1d(DFT_BUFFER_SIZE, dft_in, dft_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

void aplay::apply_filters_dft(void)
{
    int i, j = 0;
    for (i = 0; j < /*count*/10; i++)
        /*dft_in[i][0] = (double) ((data[j++] << 8) | data[j++]);*/
        //data[j++] *= 0.6;
        ;

    /* TODO */
    //fftw_execute(dft_plan);
}

aplay::~aplay()
{
    snd_pcm_close(handle);
    handle = NULL;
    free(audiobuf);
    snd_output_close(log);
    snd_config_update_free_global();
}

aplay::aplay(const char* pcm_name) :
    writei_func(snd_pcm_writei), readi_func(snd_pcm_readi),
    writen_func(snd_pcm_writen), readn_func(snd_pcm_readn),
    paused(0),
    command("aplay"),
    timelimit(0), // TODO: add a way to customize these
    file_type(FORMAT_WAVE),
    open_mode(0),
    stream(SND_PCM_STREAM_PLAYBACK),
    interleaved(1),
    in_aborting(0),
    chunk_size(1024),
    period_time(0),
    buffer_time(0),
    period_frames(0),
    buffer_frames(0),
    avail_min(-1),
    start_delay(0), // TODO: add a way to customize this
    stop_delay(0),
    monotonic(0),
    can_pause(0),
    fatal_errors(0),
    vumeter(VUMETER_NONE),
    buffer_pos(0),
    test_position(0),
    test_coef(8),
    test_nowait(0),
    sound_memory(0)
{
    audiobuf = (u_char *) malloc(chunk_size); 
    if (audiobuf == NULL)
        throw audio::bad_alloc();

    snd_pcm_info_t *info;

    rhwparams.format = SND_PCM_FORMAT_S16_LE;
    rhwparams.rate = 44100;
    rhwparams.channels = 2;
    hwparams = rhwparams;

    if (snd_output_stdio_attach(&log, stderr, 0) < 0)
        throw audio::connection_failed();

    if (snd_pcm_open(&handle, pcm_name, stream, open_mode) < 0)
        throw audio::connection_failed();

    snd_pcm_info_alloca(&info);

    if (snd_pcm_info(handle, info) < 0)
        throw audio::info_retrieve_failed();

    playback();
}

//aplay::aplay(const aplay &ap) :
    //sound_memory(ap.sound_memory)
//{ [> TODO <] }

void aplay::init(const char *filename)
{
}

void aplay::init(FILE *file)
{
    m_safe_read = safe_read_file;
}

void aplay::init(memory_ref &mem)
{
    m_safe_read = safe_read_memory;
}

void aplay::init()
{
}

void aplay::seek(long miliseconds)
{
}

void aplay::toggle_pause(void)
{
}

void aplay::show_available_sample_formats(snd_pcm_hw_params_t* params)
{
    int format;

    fprintf(stderr, "Available formats:\n");
    for (format =  0; format <= SND_PCM_FORMAT_LAST; format++) {
        if (snd_pcm_hw_params_test_format(handle, params, (snd_pcm_format_t) format) == 0)
            fprintf(stderr, "- %s\n", snd_pcm_format_name((snd_pcm_format_t) format));
    }
}

void aplay::prg_exit(int code)
{
    // TODO
}

int aplay::setup_chmap(void)
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

    hw_map = (unsigned int *) calloc(hwparams.channels, sizeof(int));
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

void aplay::xrun(void)
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
        if ((res = snd_pcm_prepare(handle))<0) {
            error(_("xrun: prepare error: %s"), snd_strerror(res));
            prg_exit(EXIT_FAILURE);
        }
        return;     /* ok, data should be accepted again */
    } if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
        if (stream == SND_PCM_STREAM_CAPTURE) {
            fprintf(stderr, _("capture stream format change? attempting recover...\n"));
            if ((res = snd_pcm_prepare(handle))<0) {
                error(_("xrun(DRAINING): prepare error: %s"), snd_strerror(res));
                prg_exit(EXIT_FAILURE);
            }
            return;
        }
    }
    error(_("read/write error, state = %s"), snd_pcm_state_name(snd_pcm_status_get_state(status)));
    prg_exit(EXIT_FAILURE);
}

void aplay::compute_max_peak(u_char *data, size_t count)
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
}

void aplay::do_test_position(void)
{
    static long counter = 0;
    static time_t tmr = -1;
    time_t now;
    float availsum, delaysum, samples;
    snd_pcm_sframes_t maxavail, maxdelay;
    snd_pcm_sframes_t minavail, mindelay;
    snd_pcm_sframes_t badavail = 0, baddelay = 0;
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
    }
}

u_char *aplay::remap_data(u_char *data, size_t count)
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
        tmp = (u_char *) malloc(chunk_bytes);
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

u_char **aplay::remap_datav(u_char **data, size_t count)
{
    static u_char **tmp;
    unsigned int ch;

    if (!hw_map)
        return data;

    if (!tmp) {
        tmp = (u_char **) malloc(sizeof(*tmp) * hwparams.channels);
        if (!tmp) {
            error(_("not enough memory"));
            exit(1);
        }
        for (ch = 0; ch < hwparams.channels; ch++)
            tmp[ch] = data[hw_map[ch]];
    }
    return tmp;
}

ssize_t aplay::pcm_write(u_char *data, size_t count)
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

off64_t aplay::calc_count(void)
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

void aplay::set_params(void) noexcept(false)
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
    if (err < 0)
        throw audio::broken_pcm_confg();
    if (interleaved)
        err = snd_pcm_hw_params_set_access(handle, params,
                           SND_PCM_ACCESS_RW_INTERLEAVED);
    else
        err = snd_pcm_hw_params_set_access(handle, params,
                           SND_PCM_ACCESS_RW_NONINTERLEAVED);
    if (err < 0)
        throw audio::access_type_not_available();
    err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
    if (err < 0)
        throw audio::sample_format_not_available();
    err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
    if (err < 0)
        throw audio::channel_count_not_available();

    rate = hwparams.rate;
    err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
    assert(err >= 0);
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
    if (err < 0)
        throw audio::unable_to_install_hw_params();
    snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
    snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
    if (chunk_size == buffer_size)
        throw audio::invalid_period();
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

    if (snd_pcm_sw_params(handle, swparams) < 0)
        throw audio::unable_to_install_sw_params();

    if (setup_chmap())
        throw audio::chmap_setup_failed();

    bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
    bits_per_frame = bits_per_sample * hwparams.channels;
    chunk_bytes = chunk_size * bits_per_frame / 8;
    audiobuf = (u_char *) realloc(audiobuf, chunk_bytes);
    if (audiobuf == NULL)
        throw audio::bad_alloc();

    /* stereo VU-meter isn't always available... */
    if (vumeter == VUMETER_STEREO) {
        if (hwparams.channels != 2 || !interleaved)
            vumeter = VUMETER_MONO;
    }

    /* show mmap buffer arragment */
    buffer_frames = buffer_size;    /* for position test */
}

void aplay::playback_go(size_t loaded, off64_t count)
{
    int l, r;
    off64_t written = 0;
    off64_t c;
    void *rdpass;
    if (sound_file == NULL)
        rdpass = (void *) (&sound_memory);
    else
        rdpass = (void *) sound_file;

    set_params();

    while (loaded > chunk_bytes && written < count) {
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
            r = m_safe_read(rdpass , audiobuf + l, c);
            if (r < 0)
                throw audio::bad_read();
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
    snd_pcm_nonblock(handle, 0);
}

void aplay::playback(void)
{
    ssize_t dtawave;
    pbrec_count = LLONG_MAX;
    fdcount = 0;
    /* read bytes for WAVE-header */
    pbrec_count = calc_count();
    playback_go(dtawave, pbrec_count);
    if (fd != 0)
        close(fd);
}

