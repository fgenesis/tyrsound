#include "tyrsound_internal.h"
#include "OggDecoder.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <string.h>

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DECODER(OggDecoder);


static size_t read_wrap(void *dst, size_t size, size_t nmemb, void *datasource)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)datasource;
    return (size_t)strm->read(dst, size, nmemb, strm->user);
}

static int seek_wrap(void *datasource, ogg_int64_t offset, int whence)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)datasource;
    return strm->seek(strm->user, offset, whence);
}

static int close_wrap(void *datasource)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)datasource;
    return strm->close(strm->user);
}

static long tell_wrap(void *datasource)
{
     tyrsound_Stream *strm = (tyrsound_Stream*)datasource;
     return (long)strm->tell(strm->user);
}

static const ov_callbacks stream_callbacks =
{
    read_wrap,
    seek_wrap,
    close_wrap,
    tell_wrap
};


struct OggDecoderState
{
    OggVorbis_File vf;
    tyrsound_Stream strm;
};


bool OggDecoder::checkMagic(const unsigned char *magic, size_t size)
{
    return !memcmp(magic, "OggS", 4);
}


OggDecoder::OggDecoder(void *state, const tyrsound_Format& fmt)
: _state(state)
, _loopPoint(-1.0f)
, _eof(false)
, _loopCount(0)
{
    // 16 bits if not specified
    unsigned int bits = fmt.sampleBits == 0 ? 16 : fmt.sampleBits;
    _sampleWordSize = (bits <= 8 ? 1 : 2);
    _totaltime = (float)ov_time_total(&((OggDecoderState*)state)->vf, -1);
    _seekable = ov_seekable(&((OggDecoderState*)state)->vf) != 0;
    _fmt = fmt;
    vorbis_info *vi = ov_info(&((OggDecoderState*)state)->vf, -1);
    _fmt.channels = vi->channels;
    _fmt.hz = vi->rate;
    _fmt.sampleBits = bits;
}

OggDecoder::~OggDecoder()
{
    // Stream is closed by ov_clear()
    ov_clear(&((OggDecoderState*)_state)->vf);
    Free(_state);
}

OggDecoder *OggDecoder::create(const tyrsound_Format& fmt, tyrsound_Stream strm)
{
    OggDecoderState *state = (OggDecoderState*)Alloc(sizeof(OggDecoderState));
    if(!state)
        return NULL;

    state->strm = strm;
    
    if(ov_test_callbacks(&state->strm, &state->vf, NULL, 0, stream_callbacks))
    {
        Free(state);
        return NULL;
    }
    if(ov_test_open(&state->vf))
    {
        Free(state);
        return NULL;
    }

    void *mem = Alloc(sizeof(OggDecoder));
    if(!mem)
    {
        Free(state);
        return NULL;
    }

    OggDecoder *decoder = new(mem) OggDecoder(state, fmt);
    return decoder;
}

size_t OggDecoder::fillBuffer(void *buf, size_t size)
{
    OggDecoderState *state = (OggDecoderState*)_state;
    char *dst = (char*)buf;
    int dummy;
    size_t totalRead = 0;
    while(totalRead < size)
    {
        long bytesRead = ov_read(&state->vf, dst + totalRead, size - totalRead, _fmt.bigendian, _sampleWordSize, _fmt.signedSamples, &dummy);
        if(bytesRead < 0)
            break;
        else if(bytesRead == 0)
        {
            if(!_loopCount)
            {
                _eof = true;
                break;
            }

            if(_loopPoint >= 0)
            {
                // TODO: callback
                ov_time_seek(&state->vf, _loopPoint);
                if(_loopCount > 0)
                    --_loopCount;
            }
            else
            {
                _eof = true;
                break;
            }
        }
        else
            totalRead += bytesRead;
    }
    return totalRead;
}

tyrsound_Error OggDecoder::seek(float seconds)
{
    _eof = false;
    return ov_time_seek(&((OggDecoderState*)_state)->vf, seconds)
        ? TYRSOUND_ERR_UNSPECIFIED
        : TYRSOUND_ERR_OK;
}

tyrsound_Error OggDecoder::setLoop(float seconds, int loops)
{
    if(_seekable)
    {
        _loopPoint = seconds;
        _loopCount = loops;
        return TYRSOUND_ERR_OK;
    }
    return TYRSOUND_ERR_UNSUPPORTED;
}

float OggDecoder::getLoopPoint()
{
    return _loopPoint;
}


float OggDecoder::getLength()
{
    return _totaltime;
}

float OggDecoder::tell()
{
    return (float)ov_time_tell(&((OggDecoderState*)_state)->vf);
}

bool OggDecoder::isEOF()
{
    return _eof;
}

void OggDecoder::getFormat(tyrsound_Format *fmt)
{
    *fmt = _fmt;
}


#include "tyrsound_end.h"

