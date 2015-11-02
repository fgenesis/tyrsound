#ifndef TYRSOUND_WAV_DECODER_H
#define TYRSOUND_WAV_DECODER_H

#include "tyrDecoderBase.h"

#include "tyrsound_begin.h"


class WavDecoder : public DecoderBase
{
protected:
    WavDecoder(const tyrsound_Stream& strm, const tyrsound_Format& fmt, unsigned int datasize);
    virtual ~WavDecoder();

public:
    static WavDecoder *create(const tyrsound_Format& fmt, const tyrsound_Stream& strm);
    static const char *getName() { return "WAV"; }
    static void staticInit() {}
    static void staticShutdown() {}
    static bool checkMagic(const unsigned char *magic, size_t size);

    virtual size_t fillBuffer(void *buf, size_t size);
    virtual tyrsound_Error seekSample(tyrsound_uint64 sample);
    virtual tyrsound_uint64 tellSample();
    virtual tyrsound_Error setLoop(float seconds, int loops);
    virtual float getLoopPoint();
    virtual bool isEOF();

protected:
    tyrsound_Stream _strm;
    float _loopPoint;
    int _loopCount;
    bool _eof;
};

#include "tyrsound_end.h"

#endif

