
// WARNING: The code in this file SUCKS.

// Blame the libFLAC API, this crap doesn't even offer a function like
// "decode N bytes and store them in this buffer" like EVERY other decoder I've seen.
// May whoever has designed this API be eaten by a thousand hungry orcs.

#include "tyrsound_internal.h"
#include "FlacDecoder.h"
#include <FLAC/all.h>
#include <string.h>
#include <stdio.h>

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DECODER(FlacDecoder);


struct FlacDecoderState
{
    FLAC__StreamDecoder *flac;
    tyrsound_Stream strm;

    // decoder work-vars
    char *writebuf; // temp. storage for the pointer supplied via FlacDecoder::fillBuffer()
    size_t todosize; // size of the remaining sound buffer (writebuf)
    size_t writtensize; // bytes written to writebuf, per decode call

    char *samplebuf; // Excess samples that don't fit into writebuf will go here.
    size_t buffered; // bytes readable from samplebuf
    size_t samplebufsize; // total samplebuf size, in bytes
    size_t samplebufpos; // current read position in samplebuf

    // set via flac metadata
    unsigned int maxFrameSize;
    unsigned int sampleBitSize;
    unsigned int sampleRate;
    unsigned int channels;
    unsigned int bytesPerSample;
    tyrsound_uint64 totalSamples;
};


static FLAC__StreamDecoderReadStatus read_wrap(const FLAC__StreamDecoder *, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
    FlacDecoderState *state = (FlacDecoderState*)client_data;
    tyrsound_Stream& strm = state->strm;

    *bytes = (size_t)strm.read(buffer, 1, *bytes, strm.user);
    return *bytes ? FLAC__STREAM_DECODER_READ_STATUS_CONTINUE : FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
}

static FLAC__StreamDecoderSeekStatus seek_wrap(const FLAC__StreamDecoder *, FLAC__uint64 absolute_byte_offset, void *client_data)
{
    FlacDecoderState *state = (FlacDecoderState*)client_data;
    tyrsound_Stream& strm = state->strm;

    int res = strm.seek(strm.user, absolute_byte_offset, SEEK_SET);
    return res < 0 ? FLAC__STREAM_DECODER_SEEK_STATUS_ERROR : FLAC__STREAM_DECODER_SEEK_STATUS_OK;
}

static FLAC__StreamDecoderTellStatus tell_wrap(const FLAC__StreamDecoder *, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
    FlacDecoderState *state = (FlacDecoderState*)client_data;
    tyrsound_Stream& strm = state->strm;

    tyrsound_int64 pos = strm.tell(strm.user);
    if(pos >= 0)
    {
        *absolute_byte_offset = (FLAC__uint64)pos;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    }
    return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
}

static FLAC__bool eof_wrap(const FLAC__StreamDecoder *, void *client_data)
{
    FlacDecoderState *state = (FlacDecoderState*)client_data;
    tyrsound_Stream& strm = state->strm;

    return !strm.remain(strm.user);
}

static FLAC__StreamDecoderLengthStatus length_wrap(const FLAC__StreamDecoder *, FLAC__uint64 *stream_length, void *client_data)
{
    FlacDecoderState *state = (FlacDecoderState*)client_data;
    tyrsound_Stream& strm = state->strm;

    tyrsound_int64 remain = strm.remain(strm.user);
    tyrsound_int64 done = strm.tell(strm.user);
    if(remain < 0 || done < 0)
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;

    *stream_length = remain + done;
    return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}



static void s_metadataCallback(const FLAC__StreamDecoder *, const FLAC__StreamMetadata *metadata, void *client_data)
{
    if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
    {
        FlacDecoderState *state = (FlacDecoderState*)client_data;
        // "FLAC specifies a minimum block size of 16 and a maximum block size of 65535,
        // meaning the bit patterns corresponding to the numbers 0-15 in the minimum blocksize
        // and maximum blocksize fields are invalid."
        // -- So we just assume the worst and keep it at the pre-set spec maximum if the values are derp.
        if(metadata->data.stream_info.max_framesize >= 16)
            state->maxFrameSize = metadata->data.stream_info.max_framesize;

        state->sampleBitSize = metadata->data.stream_info.bits_per_sample;
        state->channels = metadata->data.stream_info.channels;
        state->sampleRate = metadata->data.stream_info.sample_rate;
        // > 16 bit samples are casted down to 16 bit (see s_decodeToBuffer())
        state->bytesPerSample = state->channels * (Max(16u, state->sampleBitSize) / 8);
        state->totalSamples = metadata->data.stream_info.total_samples;
    }
}

static void s_errorCallback(const FLAC__StreamDecoder *, FLAC__StreamDecoderErrorStatus status, void *)
{
#if TYRSOUND_IS_DEBUG
    printf("FLAC decoder error: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
#endif
}


bool FlacDecoder::checkMagic(const unsigned char *magic, size_t size)
{
    return !memcmp(magic, "fLaC", 4);
}


FlacDecoder::FlacDecoder(void *pstate, const tyrsound_Format& fmt)
: _state(pstate)
, _loopPoint(-1.0f)
, _eof(false)
, _loopCount(0)
, _fmt(fmt)
{
    FlacDecoderState *state = (FlacDecoderState*)pstate;

    _seekable = state->strm.seek && state->strm.remain && state->strm.tell;
    _totaltime = !state->totalSamples ? -1.0f : (state->totalSamples / float(fmt.hz));
}

FlacDecoder::~FlacDecoder()
{
    FlacDecoderState *state = (FlacDecoderState*)_state;
    state->strm.close(state->strm.user);
    Free(state->samplebuf);

    FLAC__StreamDecoder *flac = state->flac;
    FLAC__stream_decoder_finish(flac);
    FLAC__stream_decoder_delete(flac);

    Free(_state);
}

template <typename T, unsigned SH> static size_t s_decodeToBuffer(T *outbuf, size_t size, const FLAC__int32 *const buffer[], unsigned int channels, unsigned int blocksize, unsigned int bytesPerSample, unsigned int *blocksDone)
{
    const unsigned int offs = *blocksDone;
    const unsigned int remainblocks = blocksize - offs;
    unsigned int i;
    for(i = 0; size && i < remainblocks; ++i, size -= bytesPerSample)
        for(unsigned int c = 0; c < channels; ++c)
            *outbuf++ = static_cast<T>(buffer[c][i+offs] >> SH); // signed shift

    *blocksDone = i + offs;
    size_t written = i * bytesPerSample;
    return written;
}

template <typename T, unsigned SH> static size_t s_decodeMain(FlacDecoderState *state, const FLAC__int32 *const buffer[], unsigned int blocksize)
{
    if(state->samplebufpos || state->buffered)
        breakpoint();

    size_t written = 0;
    unsigned int blocksDone = 0;
    // First, fill the audio buffer as much as possible.
    if(state->todosize)
    {
        written = s_decodeToBuffer<T, SH>((T*)state->writebuf, state->todosize, buffer, state->channels, blocksize, state->bytesPerSample, &blocksDone);
        state->todosize -= written;
        state->writtensize += written;
        if(blocksDone == blocksize) // If the whole frame was done, we can exit now
            return written;
    }


    size_t buffered = s_decodeToBuffer<T, SH>((T*)state->samplebuf, state->samplebufsize, buffer, state->channels, blocksize, state->bytesPerSample, &blocksDone);
    if(blocksize != blocksDone)
        breakpoint();

    written += buffered;
    state->buffered += buffered;

    return written;
}

static FLAC__StreamDecoderWriteStatus s_writeCallback(const FLAC__StreamDecoder *, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
    FlacDecoderState *state = (FlacDecoderState*)client_data;

    int done = 0;
    unsigned int blocksize = frame->header.blocksize;

    switch(state->sampleBitSize)
    {
        case 8:
            done = s_decodeMain<char, 0>(state, buffer, blocksize);
            break;

        case 16:
            done = s_decodeMain<short, 0>(state, buffer, blocksize);
            break;

        // Downcast higher qualities by shifting
        // This is also assumed in s_metadataCallback()
        case 24:
            done = s_decodeMain<short, 8>(state, buffer, blocksize);
            break;
        case 32:
            done = s_decodeMain<short, 16>(state, buffer, blocksize);
            break;

        default:
            return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    return done ? FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE : FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

}

FlacDecoder *FlacDecoder::create(const tyrsound_Format& /*not used*/, tyrsound_Stream strm)
{
    FlacDecoderState *state = (FlacDecoderState*)Alloc(sizeof(FlacDecoderState));
    if(!state)
        return NULL;

    FLAC__StreamDecoder *flac = FLAC__stream_decoder_new();
    if(!flac)
    {
        Free(state);
        return NULL;
    }

    memset(state, 0, sizeof(*state));

    state->maxFrameSize = 65535; // By FLAC specification. Changed when reading metadata.
    state->strm = strm;
    state->flac = flac;

    FLAC__stream_decoder_set_md5_checking(state->flac, 0);

    FLAC__StreamDecoderInitStatus initStatus = FLAC__stream_decoder_init_stream(
        state->flac, read_wrap, seek_wrap, tell_wrap, length_wrap, eof_wrap,
        s_writeCallback, s_metadataCallback, s_errorCallback, state
    );

    int metadataDone = 0;
    if(initStatus == FLAC__STREAM_DECODER_INIT_STATUS_OK)
        metadataDone = FLAC__stream_decoder_process_until_end_of_metadata(state->flac);

    // Because of libFLAC's purely callback-oriented API, we need to be able to buffer whatever comes in.
    // Therefore reserve a large blob of memory up front so we won't run into trouble later.
    // FIXME: this calculation is not correct, the buffer is too large
    state->samplebufsize = state->maxFrameSize * state->channels * (state->sampleBitSize / 8);
    if(!state->sampleRate
        || !metadataDone
        || !(state->samplebuf = (char*)Alloc(state->samplebufsize)))
    {
        FLAC__stream_decoder_delete(state->flac);
        Free(state);
        return NULL;
    }

    void *mem = Alloc(sizeof(FlacDecoder));
    if(!mem)
    {
        Free(state->samplebuf);
        FLAC__stream_decoder_delete(state->flac);
        Free(state);
        return NULL;
    }

    tyrsound_Format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.bigendian = isBigEndian();
    fmt.channels = state->channels;
    fmt.hz = state->sampleRate;
    fmt.sampleBits = state->sampleBitSize;
    fmt.signedSamples = 1;

    FlacDecoder *decoder = new(mem) FlacDecoder(state, fmt);

    return decoder;
}


size_t FlacDecoder::fillBuffer(void *buf, size_t size)
{
    char *dstbuf = (char*)buf;
    FlacDecoderState *state = (FlacDecoderState*)_state;
    size_t totalWritten = 0;

    // Make sure the buffer end is sample aligned for all channels
    size -= size % state->bytesPerSample;

    // Copy whatever is in the decoded buffer to the output buffer, up to size.
    // Might not be able to supply size bytes, in that case we'll continue below.
    size_t copysize = Min(size, (size_t)state->buffered);
    if(copysize % state->bytesPerSample)
        breakpoint();
    if(copysize)
    {
        memcpy(dstbuf, &state->samplebuf[state->samplebufpos], copysize);
        state->buffered -= copysize;
        state->samplebufpos += copysize;
        totalWritten += copysize;
        size -= copysize;
        dstbuf += copysize;
    }

    // If we're here, samplebuf is used up.
    if(size)
    {
        state->writebuf = dstbuf;
        state->todosize = size;
        state->samplebufpos = 0; // start reading at beginning

        if(state->buffered)
            breakpoint();
        do
        {
            state->writtensize = 0;

            // Buffer wasn't enough, need to decode more.
            // The rest of the logic happens in s_writeCallback().
            int proc = FLAC__stream_decoder_process_single(state->flac);

            state->writebuf += state->writtensize;
            totalWritten += state->writtensize;

            FLAC__StreamDecoderState s = FLAC__stream_decoder_get_state(state->flac);
            switch(s)
            {
                case FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
                case FLAC__STREAM_DECODER_READ_FRAME:
                    break;

                case FLAC__STREAM_DECODER_END_OF_STREAM:
                {
                    if(!_loopCount)
                    {
                        _eof = true;
                        goto end;
                    }

                    if(_loopPoint >= 0)
                    {
                        if(seek(_loopPoint) < TYRSOUND_ERR_OK)
                        {
                            _eof = true;
                            goto end;
                        }
                        if(_loopCount > 0)
                            --_loopCount;
                    }
                    else
                    {
                        _eof = true;
                        goto end;
                    }
                }
                break;

                default:
                {
                    _eof = true;
                    goto end;
                }
                break;
            }
        }
        while(state->todosize);
    }

end:

    return totalWritten;
}

// FIXME: just guessing here, the FLAC doc is somewhat hard to understand
tyrsound_Error FlacDecoder::seek(float seconds)
{
    FlacDecoderState *state = (FlacDecoderState*)_state;
    FLAC__bool seeked = FLAC__stream_decoder_seek_absolute(state->flac, FLAC__uint64(seconds * state->sampleRate));
    FLAC__bool flushOk = 1;
    if(FLAC__stream_decoder_get_state(state->flac) == FLAC__STREAM_DECODER_SEEK_ERROR)
        flushOk = FLAC__stream_decoder_flush(state->flac);

    if(!seeked && !flushOk)
        _eof = true;

    if(seeked)
    {
        _eof = false;
        return TYRSOUND_ERR_OK;
    }

    return flushOk ? TYRSOUND_ERR_UNSUPPORTED : TYRSOUND_ERR_UNSPECIFIED;
}

tyrsound_Error FlacDecoder::setLoop(float seconds, int loops)
{
    if(_seekable)
    {
        _loopPoint = seconds;
        _loopCount = loops;
        return TYRSOUND_ERR_OK;
    }
    return TYRSOUND_ERR_UNSUPPORTED;
}

float FlacDecoder::getLoopPoint()
{
    return _loopPoint;
}


float FlacDecoder::getLength()
{
    return _totaltime;
}

float FlacDecoder::tell()
{
    return -1; //(float)ov_time_tell(&((FlacDecoderState*)_state)->vf);
}

bool FlacDecoder::isEOF()
{
    return _eof;
}

void FlacDecoder::getFormat(tyrsound_Format *fmt)
{
    *fmt = _fmt;
}


#include "tyrsound_end.h"

