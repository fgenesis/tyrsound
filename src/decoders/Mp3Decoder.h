#ifndef TYRSOUND_MP3_DECODER_H
#define TYRSOUND_MP3_DECODER_H

#include "tyrDecoderBase.h"

#include "tyrsound_begin.h"


class Mp3Decoder : public DecoderBase
{
protected:
    Mp3Decoder(void *state, const tyrsound_Format& fmt, bool seekable);
    virtual ~Mp3Decoder();

public:
    static Mp3Decoder *create(const tyrsound_Format& fmt, tyrsound_Stream strm);
    static const char *getName() { return "MP3"; }
    static void staticInit();
    static void staticShutdown();
    static bool checkMagic(const unsigned char *magic, size_t size);

    virtual size_t fillBuffer(void *buf, size_t size);
    virtual tyrsound_Error seekSample(tyrsound_uint64 sample);
    virtual tyrsound_uint64 tellSample();
    virtual tyrsound_Error setLoop(float seconds, int loops);
    virtual float getLoopPoint();
    virtual bool isEOF();

private:

    void *_state;
    float _loopPoint;
    int _loopCount;
    bool _seekable;
    bool _eof;

    static void *s_dynHandle;
    static bool s_enabled;
};



#include "tyrsound_end.h"
#endif
