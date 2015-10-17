#include "aplay.h"

void aplay::plan_dft(void)
{
    dft_in = (fftw_complex *) fftw_malloc(sizeof(*dft_in) * DFT_BUFFER_SIZE);
    dft_out = (fftw_complex *) fftw_malloc(sizeof(*dft_out) * DFT_BUFFER_SIZE);
    dft_plan = fftw_plan_dft_1d(DFT_BUFFER_SIZE, dft_in, dft_out, FFTW_FORWARD, FFTW_ESTIMATE);
}

void aplay::apply_filters_dft(void)
{
    int i, j = 0;
    for (i = 0; j < /*count*/10; i++)
        /*dft_in[i][0] = (double) ((data[j++] << 8) | data[j++]);*/
        //data[j++] *= 0.6;

    /* TODO */
    fftw_execute(dft_plan);
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

ssize_t aplay::safe_read(int fd, void *buf, size_t count)
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

size_t aplay::test_wavefile_read(int fd, u_char *buffer, size_t *size, size_t reqsize, int line)
{
    if (*size >= reqsize)
        return *size;
    if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
        error(_("read error (called from line %i)"), line);
        prg_exit(EXIT_FAILURE);
    }
    return *size = reqsize;
}

/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 *                            == 0 if not
 * Value returned is bytes to be discarded.
 */
ssize_t aplay::test_wavefile(int fd, u_char *_buffer, size_t size)
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
    else
        return -1;
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
                snd_pcm_format_name((snd_pcm_format_t) native_format));
        hwparams.format = (snd_pcm_format_t) native_format;
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
                    snd_pcm_format_name((snd_pcm_format_t) native_format));
            hwparams.format = (snd_pcm_format_t) native_format;
            break;
        case 4:
            if (big_endian)
                native_format = SND_PCM_FORMAT_S24_BE;
            else
                native_format = SND_PCM_FORMAT_S24_LE;
            if (hwparams.format != DEFAULT_FORMAT &&
                hwparams.format != native_format)
                fprintf(stderr, _("Warning: format is changed to %s\n"),
                    snd_pcm_format_name((snd_pcm_format_t) native_format));
            hwparams.format = (snd_pcm_format_t) native_format;
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
                        hwparams.format = (snd_pcm_format_t) native_format;
        } else if (format == WAV_FMT_IEEE_FLOAT) {
            if (big_endian)
                native_format = SND_PCM_FORMAT_FLOAT_BE;
            else
                native_format = SND_PCM_FORMAT_FLOAT_LE;
            hwparams.format = (snd_pcm_format_t) native_format;
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

void aplay::set_params(void)
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
        snd_pcm_access_mask_t *mask = (snd_pcm_access_mask_t *)alloca(snd_pcm_access_mask_sizeof());
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

    bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
    bits_per_frame = bits_per_sample * hwparams.channels;
    chunk_bytes = chunk_size * bits_per_frame / 8;
    audiobuf = (u_char *) realloc(audiobuf, chunk_bytes);
    if (audiobuf == NULL) {
        error(_("not enough memory"));
        prg_exit(EXIT_FAILURE);
    }
    /*fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);*/

    /* stereo VU-meter isn't always available... */
    if (vumeter == VUMETER_STEREO) {
        if (hwparams.channels != 2 || !interleaved)
            vumeter = VUMETER_MONO;
    }

    /* show mmap buffer arragment */
    if (mmap_flag) {
        const snd_pcm_channel_area_t *areas;
        snd_pcm_uframes_t offset, size = chunk_size;
        int i;
        err = snd_pcm_mmap_begin(handle, &areas, &offset, &size);
        if (err < 0) {
            error(_("snd_pcm_mmap_begin problem: %s"), snd_strerror(err));
            prg_exit(EXIT_FAILURE);
        }
        for (i = 0; i < (signed) hwparams.channels; i++)
            fprintf(stderr, "mmap_area[%i] = %p,%u,%u (%u)\n", i, areas[i].addr, areas[i].first, areas[i].step, snd_pcm_format_physical_width(hwparams.format));
        /* not required, but for sure */
        snd_pcm_mmap_commit(handle, offset, 0);
    }

    buffer_frames = buffer_size;    /* for position test */
}

void aplay::playback_go(int fd, size_t loaded, off64_t count, int rtype, char *name)
{
    int l, r;
    off64_t written = 0;
    off64_t c;

    set_params();

    while (loaded > chunk_bytes && written < count && !in_aborting) {
        //if (filters_avail)
            //do_apply_filters(audiobuf + written, chunk_size);

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

void aplay::playback(char *filename)
{
    size_t dta;
    ssize_t dtawave;
    char *name;
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

void aplay::playbackv(char **names, unsigned int count)
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
        names = (char **) malloc(sizeof(*names) * channels);
        for (channel = 0; channel < channels; ++channel) {
            names[channel] = (char *) malloc(len);
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
