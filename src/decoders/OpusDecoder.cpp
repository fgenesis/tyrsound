#include "tyrsound_internal.h"
#include "OpusDecoder.h"
#include <opusfile.h>

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DECODER(OpusDecoder);


static int read_wrap(void *streamp, unsigned char *out, int size)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)streamp;
    return (int)strm->read(out, 1, size, strm->user);
}

static int seek_wrap(void *streamp, opus_int64 offset, int whence)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)streamp;
    return strm->seek(strm->user, offset, whence);
}

static int close_wrap(void *streamp)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)streamp;
    int res = strm->close(strm->user);
    Free(strm);
    return res;
}

static opus_int64 tell_wrap(void *streamp)
{
     tyrsound_Stream *strm = (tyrsound_Stream*)streamp;
     return strm->tell(strm->user);
}

static const OpusFileCallbacks stream_callbacks =
{
    read_wrap,
    seek_wrap,
    tell_wrap,
    close_wrap
};

#define OPUS ((OggOpusFile*)_state)

OpusDecoder::OpusDecoder(void *state, const tyrsound_Format& fmt)
: _state(state)
, _loopPoint(-1.0f)
, _eof(false)
, _loopCount(0)
{
    opus_int64 pcmlen = op_pcm_total(OPUS, -1);
    _totaltime = pcmlen < 0 ? -1.0f : float((double)pcmlen / 48000.0);
    _seekable = op_seekable(OPUS) != 0;
    _fmt = fmt;
    const OpusHead *oh = op_head(OPUS, -1);
    _fmt.channels = oh->channel_count ? oh->channel_count : 1;
    _fmt.hz = 48000; // oh->input_sample_rate; // 48k is opus internal format
    // TODO: check whether hardware/output device really supports 48k playback
    _fmt.sampleBits = 16;
    _fmt.signedSamples = 1;
    _fmt.bigendian = isBigEndian();
}

OpusDecoder::~OpusDecoder()
{
    op_free(OPUS);
}

bool OpusDecoder::checkMagic(const unsigned char *magic, size_t size)
{
    return op_test(NULL, (const unsigned char*)magic, size) >= 0;
}

OpusDecoder *OpusDecoder::create(const tyrsound_Format& fmt, tyrsound_Stream strm)
{
    tyrsound_Stream *streamp = (tyrsound_Stream*)Alloc(sizeof(tyrsound_Stream));
    if(!streamp)
        return NULL;
    // Stream is closed + deleted by op_free() in dtor
    *streamp = strm;

    int err = 0;
    OggOpusFile *state = op_test_callbacks(streamp, &stream_callbacks, NULL, 0, &err);
    if(!state || err < 0)
    {
        Free(streamp);
        return NULL;
    }

    if(op_test_open(state))
    {
        op_free(state);
        return NULL;
    }

    void *mem = Alloc(sizeof(OpusDecoder));
    if(!mem)
    {
        op_free(state);
        return NULL;
    }

    OpusDecoder *decoder = new(mem) OpusDecoder(state, fmt);
    return decoder;
}

size_t OpusDecoder::fillBuffer(void *buf, size_t size)
{
    opus_int16 *dst = (opus_int16*)buf;
    size_t totalSamplesRead = 0;
    const bool forceStereo = _fmt.channels > 2;
    const int useChannels = forceStereo ? 2 : _fmt.channels;
    const size_t samplesTodo = size / sizeof(opus_int16);

    // FIXME: From the manual: "The number of channels returned can change from link to link in a chained stream."
    // need to pass tyrsound_Format ptr along with fillBuffer() instead of querying it externally everytime

    while(totalSamplesRead < samplesTodo)
    {
        // op_read() returns samples PER CHANNEL
        int samplesRead;
        if(forceStereo)
            samplesRead = op_read_stereo(OPUS, dst + totalSamplesRead, int(samplesTodo - totalSamplesRead));
        else
            samplesRead = op_read(OPUS, dst + totalSamplesRead, int(samplesTodo - totalSamplesRead), NULL);

        if(samplesRead < 0)
            break;
        else if(samplesRead == 0)
        {
            if(!_loopCount)
            {
                _eof = true;
                break;
            }

            if(_loopPoint >= 0)
            {
                // TODO: callback
                seek(_loopPoint);
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
            totalSamplesRead += samplesRead * useChannels;
    }
    return totalSamplesRead * sizeof(opus_int16); // convert back to bytes
}

tyrsound_Error OpusDecoder::seek(float seconds)
{
    _eof = false;
    opus_int64 samplepos = (opus_int64)(double(seconds) * 48000.0);
    return op_pcm_seek(OPUS, samplepos)
        ? TYRSOUND_ERR_UNSPECIFIED
        : TYRSOUND_ERR_OK;
}

tyrsound_Error OpusDecoder::setLoop(float seconds, int loops)
{
    if(_seekable)
    {
        _loopPoint = seconds;
        _loopCount = loops;
        return TYRSOUND_ERR_OK;
    }
    return TYRSOUND_ERR_UNSUPPORTED;
}

float OpusDecoder::getLoopPoint()
{
    return _loopPoint;
}


float OpusDecoder::getLength()
{
    return _totaltime;
}

float OpusDecoder::tell()
{
    opus_int64 pcmpos = op_pcm_tell(OPUS);
    return pcmpos < 0 ? -1.0f : float((double)pcmpos / 48000.0);
}

bool OpusDecoder::isEOF()
{
    return _eof;
}

void OpusDecoder::getFormat(tyrsound_Format *fmt)
{
    *fmt = _fmt;
}


#include "tyrsound_end.h"

