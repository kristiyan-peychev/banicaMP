#include "FLAC_decoder.h"
#include <cerrno>
#include <cstdlib>

#if 0
static FLAC__uint64 total_samples = 0;
static unsigned sample_rate = 0;
static unsigned channels = 0;
static unsigned bps = 0;
#endif

static inline bool write_little_endian_uint16(FILE *f, FLAC__int16 x)
{
    return fwrite(&x, 1, sizeof(FLAC__uint16), f);
}

static inline bool write_little_endian_int16(FILE *f, FLAC__int16 x)
{
    return fwrite(&x, 1, sizeof(FLAC__uint16), f);
}

static inline bool write_little_endian_uint32(FILE *f, FLAC__uint32 x)
{
    return fwrite(&x, 1, sizeof(FLAC__uint32), f);
}

flac_decoder::flac_decoder(FILE *lol) : FLAC::Decoder::File(),
    file(lol),
    memflg(false),
    out_mem(0),
    total_samples(0),
    bps(0),
    channels(0),
    sample_rate(0)
{
    FLAC__StreamDecoderInitStatus init_status = init(file);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
    }
}

flac_decoder::~flac_decoder(void) { }

::FLAC__StreamDecoderWriteStatus flac_decoder::write_callback(
        const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    if(total_samples == 0)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    if(channels != 2 || bps != 16)
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

    (this->*m_write_callback)(frame, buffer);
# if 0
    if (memflg)
        mem_write_callback(frame, buffer);
    else
        file_write_callback(frame, buffer);
# endif
}

::FLAC__StreamDecoderWriteStatus flac_decoder::mem_write_callback(
        const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    const FLAC__uint32 total_size = 
        (FLAC__uint32)(total_samples * channels * (bps/8));
    size_t i = 0, k = 0;

# if 0
    if (out->cap() < (((size_t) p - (size_t) out->begin()) + 
                (frame->header.blocksize << 1)))
        out->expand(out->cap());
# endif

    /* write decoded PCM samples */
    do {
        try {
            FLAC__int16 buf[2] = {};
            for (; i < frame->header.blocksize; i++) {
                buf[k++] = (FLAC__int16) buffer[0][i];
                buf[k++] = (FLAC__int16) buffer[1][i];
                out_mem.write((char *) buf, 2 * sizeof(*buf));
            }
            break;
        } catch (memory::write_failed &write_except) {
            out_mem.expand(out_mem.cap() * 2); // FIXME
        } catch (...) {
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
        }
    } while (1);

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

::FLAC__StreamDecoderWriteStatus flac_decoder::file_write_callback(
        const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[])
{
    const FLAC__uint32 total_size = 
        (FLAC__uint32)(total_samples * channels * (bps/8));
    size_t i = 0, k = 0;

    /* write WAVE header before we write the first frame */
    /* Correction: SCREW the header, we don't need it. */
    //if(frame->header.number.sample_number == 0) {
    //if(
    //fwrite("RIFF", 1, 4, f) < 4 ||
    //!write_little_endian_uint32(f, total_size + 36) ||
    //fwrite("WAVEfmt ", 1, 8, f) < 8 ||
    //!write_little_endian_uint32(f, 16) ||
    //!write_little_endian_uint16(f, 1) ||
    //!write_little_endian_uint16(f, (FLAC__uint16)channels) ||
    //!write_little_endian_uint32(f, sample_rate) ||
    //!write_little_endian_uint32(f, sample_rate * channels * (bps/8)) ||
    //!write_little_endian_uint16(f, (FLAC__uint16)(channels * (bps/8))) || [> block align <]
    //!write_little_endian_uint16(f, (FLAC__uint16)bps) ||
    //fwrite("data", 1, 4, f) < 4 ||
    //!write_little_endian_uint32(f, total_size)
    //) {
    //fprintf(stderr, "ERROR: write error\n");
    //return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    //}
    //}

    /* write decoded PCM samples */
    FLAC__int16 *buf = new FLAC__int16[frame->header.blocksize << 1];
    for (; i < frame->header.blocksize; i++) {
        buf[k++] = (FLAC__int16) buffer[0][i];
        buf[k++] = (FLAC__int16) buffer[1][i];
    }

    if (!fwrite(buf, sizeof(*buf), frame->header.blocksize << 1, f)) {
        delete[] buf;
        fprintf(stderr, "ERROR: write error\n");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }
    delete[] buf;

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void flac_decoder::metadata_callback(const ::FLAC__StreamMetadata *metadata)
{
    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        total_samples = metadata->data.stream_info.total_samples;
        sample_rate = metadata->data.stream_info.sample_rate;
        channels = metadata->data.stream_info.channels;
        bps = metadata->data.stream_info.bits_per_sample;
    }
}

void flac_decoder::error_callback(
        ::FLAC__StreamDecoderErrorStatus status)
{
# if 0
    fprintf(stderr, "Got error callback: %s\n", 
            FLAC__StreamDecoderErrorStatusString[status]);
# endif
    throw decode::exception();
}

bool flac_decoder::decode(FILE *outf)
{
    f = outf;
    memflg = false;
    m_write_callback = &flac_decoder::file_write_callback;
    return process_until_end_of_stream();
}

bool flac_decoder::decode(memory_ref &mem)
{
    out_mem = mem;
    memflg = true;
    m_write_callback = &flac_decoder::mem_write_callback;
    return process_until_end_of_stream();
}

