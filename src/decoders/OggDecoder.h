#ifndef TYRSOUND_OGG_DECODER_H
#define TYRSOUND_OGG_DECODER_H

#include "tyrDecoderBase.h"

#include "tyrsound_begin.h"


class OggDecoder : public DecoderBase
{
protected:
    OggDecoder(void *state, const tyrsound_Format& fmt);
    virtual ~OggDecoder();

public:
    static OggDecoder *create(const tyrsound_Format& fmt, tyrsound_Stream strm);
    static const char *getName() { return "OGG"; }
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

    void *_state;
    unsigned int _sampleWordSize;
    float _loopPoint;
    int _loopCount;
    bool _seekable;
    bool _eof;
};



#include "tyrsound_end.h"
#endif
