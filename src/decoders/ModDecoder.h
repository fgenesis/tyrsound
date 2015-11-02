#ifndef TYRSOUND_RAW_DECODER_H
#define TYRSOUND_RAW_DECODER_H

#include "tyrDecoderBase.h"

#include "tyrsound_begin.h"


class ModDecoder : public DecoderBase
{
protected:
    ModDecoder(const tyrsound_Stream& strm, const tyrsound_Format& fmt);
    virtual ~ModDecoder();

public:
    static ModDecoder *create(const tyrsound_Format& fmt, const tyrsound_Stream& strm);
    static const char *getName() { return "MOD"; }
    static void staticInit() {}
    static void staticShutdown() {}
    static bool checkMagic(const unsigned char *magic, size_t size);

    virtual size_t fillBuffer(void *buf, size_t size);
    virtual tyrsound_Error seek(float seconds);
    virtual tyrsound_Error seekSample(tyrsound_uint64 sample);
    virtual float tell();
    virtual tyrsound_uint64 tellSample();
    virtual tyrsound_Error setLoop(float seconds, int loops);
    virtual float getLoopPoint();
    virtual bool isEOF();

    void setMod(void *mod);

protected:
    size_t _readSamples(void *buf, size_t size);
    void *_mod;
    tyrsound_Stream _strm;
    float _loopPoint;
    int _loopCount;
    bool _eof;
};

#include "tyrsound_end.h"

#endif

