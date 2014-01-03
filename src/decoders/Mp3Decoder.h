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
    static void staticInit();
    static void staticShutdown();

    virtual size_t fillBuffer(void *buf, size_t size);
    virtual float getLength();
    virtual tyrsound_Error seek(float seconds);
    virtual float tell();
    virtual tyrsound_Error setLoop(float seconds, int loops);
    virtual float getLoopPoint();
    virtual bool isEOF();
    virtual void getFormat(tyrsound_Format *fmt);

private:

    void *_state;
    float _totaltime;
    tyrsound_Format _fmt;
    float _loopPoint;
    int _loopCount;
    bool _seekable;
    bool _eof;

    static void *s_dynHandle;
    static bool s_enabled;
};



#include "tyrsound_end.h"
#endif
