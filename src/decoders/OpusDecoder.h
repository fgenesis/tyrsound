#ifndef TYRSOUND_OPUS_DECODER_H
#define TYRSOUND_OPUS_DECODER_H

#include "tyrDecoderBase.h"

#include "tyrsound_begin.h"


class OpusDecoder : public DecoderBase
{
protected:
    OpusDecoder(void *state, const tyrsound_Format& fmt);
    virtual ~OpusDecoder();

public:
    static OpusDecoder *create(const tyrsound_Format& fmt, tyrsound_Stream strm);
    static void staticInit() {}
    static void staticShutdown() {}

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
};



#include "tyrsound_end.h"
#endif
