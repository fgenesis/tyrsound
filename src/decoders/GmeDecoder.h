#ifndef TYRSOUND_GME_DECODER_H
#define TYRSOUND_GME_DECODER_H

#include "tyrDecoderBase.h"

#include "tyrsound_begin.h"


class GmeDecoder : public DecoderBase
{
protected:
    GmeDecoder(void *state, const tyrsound_Format& fmt);
    virtual ~GmeDecoder();

public:
    static GmeDecoder *create(const tyrsound_Format& fmt, tyrsound_Stream strm);
    static const char *getName() { return "GME"; }
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

private:

    void *_emu;
    float _loopPoint;
    int _loopCount;
    bool _eof;
};



#include "tyrsound_end.h"
#endif
