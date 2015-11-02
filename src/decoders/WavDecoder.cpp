#include "tyrsound_internal.h"
#include "WavDecoder.h"
#include <stdio.h> // for SEEK_SET

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DECODER(WavDecoder);


bool WavDecoder::checkMagic(const unsigned char *magic, size_t size)
{
    return size > 44
        && !memcmp(magic, "RIFF", 4)        // RIFF header
        && readLE32(magic+4) > (44-8) // size of data+header-8
        && !memcmp(magic+8, "WAVEfmt ", 8)  // WAV+format header
        && readLE32(magic+16) == 16         // format header size, usually 16
        && readLE16(magic+20) == 1          // type format (1 = uncompressed). others not supported.
        && readLE16(magic+22)               // number of channels (must be at least 1)
        // ...some more data...
        && !memcmp(magic+36, "data", 4)     // start of data
        && readLE32(magic+40);              // size of data
}


WavDecoder::WavDecoder(const tyrsound_Stream& strm, const tyrsound_Format& fmt, unsigned int datasize)
: DecoderBase(_fmt)
, _strm(strm)
, _loopPoint(-1)
, _loopCount(0)
, _eof(false)
{
    c.totalsamples = datasize / (fmt.channels * (fmt.sampleBits / 8));
    c.length = c.totalsamples / (float)fmt.hz;
    _fmt.isfloat = 0;
}

WavDecoder::~WavDecoder()
{
    if(_strm.close)
        _strm.close(_strm.user);
}

WavDecoder *WavDecoder::create(const tyrsound_Format& /*unused*/, const tyrsound_Stream& strm)
{
    char wavhdr[44];
    if(!strm.read(&wavhdr[0], sizeof(wavhdr), 1, strm.user))
        return NULL;

    if(memcmp(&wavhdr[0], "RIFF", 4))
        return NULL;
    // 4: [4 byte] file size
    if(memcmp(&wavhdr[8], "WAVEfmt ", 8))
        return NULL;
    // 16: [4 byte] fmt block size
    // 20: [2 byte] type (1 = raw PCM data)

    tyrsound_Format fmt;
    fmt.channels = readLE16(&wavhdr[22]);
    fmt.hz = readLE32(&wavhdr[24]);
    //unsigned int bytesPerSec = readLE32(&wavhdr[28]);
    //unsigned int blockAln = readLE32(&wavhdr[32]);
    fmt.sampleBits = readLE16(&wavhdr[34]);

    switch(fmt.sampleBits)
    {
        case 8: case 16:
            break;
        default:
            return NULL;
    }

    fmt.signedSamples = fmt.sampleBits <= 8;

    //if(bytesPerSec != fmt.hz * fmt.channels * (fmt.sampleBits/8))
    //    return NULL;
    //if(blockAln != fmt.channels * (fmt.sampleBits/8))
    //    return NULL;

    if(memcmp(&wavhdr[36], "data", 4))
        return NULL;

    unsigned int datasize = readLE32(&wavhdr[40]);
    if(!datasize)
        return NULL;

    void *mem = Alloc(sizeof(WavDecoder));
    if(!mem)
        return NULL;

    return new(mem) WavDecoder(strm, fmt, datasize);
}

size_t WavDecoder::fillBuffer(void *buf, size_t size)
{
    size_t total = 0;
    while(!_eof && total < size)
    {
        size_t rd = (size_t)_strm.read((char*)buf + total, 1, size - total, _strm.user);
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
    return total;
}

tyrsound_Error WavDecoder::seekSample(tyrsound_uint64 sample)
{
    if(_strm.seek)
    {
        int err = _strm.seek(_strm.user, sample * _fmt.channels, SEEK_SET);
        if(!err)
        {
            _eof = false;
            return TYRSOUND_ERR_OK;
        }
        return TYRSOUND_ERR_UNSPECIFIED;
    }
    return TYRSOUND_ERR_UNSUPPORTED;
}

tyrsound_uint64 WavDecoder::tellSample()
{
    return _strm.tell ? _strm.tell(_strm.user) : 0;
}

tyrsound_Error WavDecoder::setLoop(float seconds, int loops)
{
    _loopPoint = seconds;
    _loopCount = loops;
    return TYRSOUND_ERR_OK;
}

float WavDecoder::getLoopPoint()
{
    return _loopPoint;
}

bool WavDecoder::isEOF()
{
    return _eof;
}

#include "tyrsound_end.h"
