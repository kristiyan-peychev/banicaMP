#include "MPEG_decoder.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

#define BUFF_SIZE 8388608 // 8MB

/*
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
*/

static int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* clip */
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    /* quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

MPEG_decoder::MPEG_decoder(FILE *lol)
{
    file = lol;
}

MPEG_decoder::~MPEG_decoder(void)
{
    //fclose(file);
}

static enum mad_flow input(void *data,
        struct mad_stream *stream)
{
    uberbuff *buf = static_cast<uberbuff *>(data);
    int what;

    if ((what = fread(buf->buf->start, 1, buf->buf->length, buf->rd)) > 0) {
        mad_stream_buffer(stream, buf->buf->start, buf->buf->length);
        return MAD_FLOW_CONTINUE;
    } else {
        //fprintf(stderr, "Error %d\n", what);
        return MAD_FLOW_STOP;
    }
    return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data, 
        struct mad_stream *stream,
        struct mad_frame *frame)
{
    return MAD_FLOW_CONTINUE;
}

static enum mad_flow mem_output(void *data, 
        const struct mad_header *header,
        struct mad_pcm *pcm)
{
    signed int sample;
    unsigned int nchannels, nsamples;
    const mad_fixed_t *left_ch, *right_ch;
    uberbuff *context = static_cast<uberbuff *>(data);

    nchannels = pcm->channels;
    nsamples = pcm->length;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];

    do {
        try {
            signed char tmp[4];
            while (nsamples--) {
                /* output sample(s) in 16-bit signed little-endian PCM */
                sample = scale(*left_ch++);
                tmp[0] = (sample >> 0) & 0xff;
                tmp[1] = (sample >> 8) & 0xff;
                // two-channel?
                sample = scale(*right_ch++);
                tmp[2] = (sample >> 0) & 0xff;
                tmp[3] = (sample >> 8) & 0xff;

                context->out_mem.write((const char *) tmp, 4);
            }
            break;
        } catch (memory::write_failed &write_exept) {
            context->out_mem.expand(context->out_mem.cap() * 2);
        } catch (memory::out_of_range &range_except) {
            context->out_mem.expand(context->out_mem.cap() * 2);
        } catch (...) {
            fprintf(stderr, "ERROR: FATAL EXCEPTION THROWN\n");
            return MAD_FLOW_STOP;
        }
    } while (1);

    return MAD_FLOW_CONTINUE;
}

static enum mad_flow file_output(void *data, 
        const struct mad_header *header,
        struct mad_pcm *pcm)
{
    signed int sample;
    unsigned nchannels, nsamples, ss;
    const mad_fixed_t *left_ch, *right_ch;
    uberbuff *context = static_cast<uberbuff *>(data);

    nchannels = pcm->channels;
    nsamples = pcm->length;
    ss = nsamples;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];

    signed char *to_write =
            (signed char *) malloc(nsamples * (nchannels << 1));
    signed char *tmp = to_write;
    while (nsamples--) {
        /* output sample(s) in 16-bit signed little-endian PCM */
        sample = scale(*left_ch++);
        tmp[0] = (sample >> 0) & 0xff;
        tmp[1] = (sample >> 8) & 0xff;
        // We will likely use a two-channel audio >_>
        sample = scale(*right_ch++);
        tmp[2] = (sample >> 0) & 0xff;
        tmp[3] = (sample >> 8) & 0xff;

        tmp += 4;
    }

    fwrite(to_write, sizeof(*to_write), ss * (nchannels << 1), context->f);
    free(to_write);

    return MAD_FLOW_CONTINUE;
}

bool MPEG_decoder::decode(FILE *lol)
{
    buffer buf;
    struct mad_decoder dec;
    memory_ref empty_mem(0);
    uberbuff context(&buf, lol, file, empty_mem);
    int result;

    buf.start = (unsigned char *) malloc(BUFF_SIZE * sizeof(*buf.start)); 
    buf.length = BUFF_SIZE;

    mad_decoder_init(&dec, &context, input, 0, 0, 
            file_output, error, 0);

    result = mad_decoder_run(&dec, MAD_DECODER_MODE_SYNC);

    mad_decoder_finish(&dec);
    free(buf.start);

    // FIXME
    return static_cast<bool>(!!result);
}

bool MPEG_decoder::decode(memory_ref &m)
{
    struct mad_decoder dec;
    buffer buf;
    uberbuff context(&buf, NULL, file, m);
    int result;

    buf.start = (unsigned char *) malloc(BUFF_SIZE * sizeof(*buf.start)); 
    buf.length = BUFF_SIZE;

    if (buf.start == NULL)
        throw decode::exception();

    mad_decoder_init(&dec, &context, input, 0, 0, 
            mem_output, error, 0);

    result = mad_decoder_run(&dec, MAD_DECODER_MODE_SYNC);

    mad_decoder_finish(&dec);
    free(buf.start);

    return static_cast<bool>(!!result);
}

