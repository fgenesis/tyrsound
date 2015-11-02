#include "tyrsound_internal.h"
#include "RawDecoder.h"
#include <stdio.h> // for SEEK_SET

#include "tyrsound_begin.h"

// Do NOT register this decoder! It's instantiated manually if required.
//TYRSOUND_REGISTER_DECODER(RawDecoder);

RawDecoder::RawDecoder(const tyrsound_Stream& strm, const tyrsound_Format& fmt)
: DecoderBase(fmt)
, _strm(strm)
, _loopPoint(-1)
, _loopCount(0)
, _eof(false)
{
    if(strm.remain)
    {
        tyrsound_int64 rem = _strm.remain(_strm.user);
        if(rem >= 0)
        {
            c.totalsamples = rem / (fmt.channels * (fmt.sampleBits / 8));
            c.length = c.totalsamples / float(fmt.hz);
        }
    }
}

RawDecoder::~RawDecoder()
{
    if(_strm.close)
        _strm.close(_strm.user);
}

RawDecoder *RawDecoder::create(const tyrsound_Format& fmt, const tyrsound_Stream& strm)
{
    void *mem = Alloc(sizeof(RawDecoder));
    if(!mem)
        return NULL;

    return new(mem) RawDecoder(strm, fmt);
}

size_t RawDecoder::fillBuffer(void *buf, size_t size)
{
    tyrsound_uint64 total = 0;
    while(!_eof && total < size)
    {
        tyrsound_uint64 rd = _strm.read(buf, 1, size, _strm.user);
        if(!rd)
        {
            if(!_loopCount || _loopPoint < 0)
            {
                _eof = true;
                break;
            }

            if(_loopPoint >= 0)
            {
                // TODO: callback
                if(seek(_loopPoint) < TYRSOUND_ERR_OK)
                {
                    _eof = false;
                    break;
                }
                if(_loopCount > 0)
                    --_loopCount;
            }
        }
        total += rd;
    }
    return (size_t)total;
}

tyrsound_Error RawDecoder::seekSample(tyrsound_uint64 sample)
{
    if(_strm.seek)
    {
        int err = _strm.seek(_strm.user, sample * _fmt.channels * (_fmt.sampleBits / 8), SEEK_SET);
        if(!err)
        {
            _eof = false;
            return TYRSOUND_ERR_OK;
        }
        return TYRSOUND_ERR_UNSPECIFIED;
    }
    return TYRSOUND_ERR_UNSUPPORTED;
}

tyrsound_uint64 RawDecoder::tellSample()
{
    return _strm.tell ? (_strm.tell(_strm.user) / (_fmt.channels * (_fmt.sampleBits / 8))) : 0;
}

tyrsound_Error RawDecoder::setLoop(float seconds, int loops)
{
    _loopPoint = seconds;
    _loopCount = loops;
    return TYRSOUND_ERR_OK;
}

float RawDecoder::getLoopPoint()
{
    return _loopPoint;
}

bool RawDecoder::isEOF()
{
    return _eof;
}

#include "tyrsound_end.h"
