#ifndef TYRSOUND_RAW_DECODER_H
#define TYRSOUND_RAW_DECODER_H

#include "tyrDecoderBase.h"

#include "tyrsound_begin.h"


class RawDecoder : public DecoderBase
{
protected:
    RawDecoder(const tyrsound_Stream& strm, const tyrsound_Format& fmt);
    virtual ~RawDecoder();

public:
    static RawDecoder *create(const tyrsound_Format& fmt, const tyrsound_Stream& strm);
    static const char *getName() { return "Raw"; }
    static void staticInit() {}
    static void staticShutdown() {}
    static bool checkMagic(const unsigned char *magic, size_t size) { return true; }

    virtual size_t fillBuffer(void *buf, size_t size);
    virtual float getLength();
    virtual tyrsound_Error seek(float seconds);
    virtual float tell();
    virtual tyrsound_Error setLoop(float seconds, int loops);
    virtual float getLoopPoint();
    virtual bool isEOF();
    virtual void getFormat(tyrsound_Format *fmt);

protected:
    tyrsound_Stream _strm;
    tyrsound_Format _fmt;
    float _loopPoint;
    int _loopCount;
    float _totaltime;
    bool _eof;
};

#include "tyrsound_end.h"

#endif

