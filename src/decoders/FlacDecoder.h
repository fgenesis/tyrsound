#ifndef TYRSOUND_FLAC_DECODER_H
#define TYRSOUND_FLAC_DECODER_H

#include "tyrDecoderBase.h"

#include "tyrsound_begin.h"


class FlacDecoder : public DecoderBase
{
protected:
    FlacDecoder(void *state, const tyrsound_Format& fmt);
    virtual ~FlacDecoder();

public:
    static FlacDecoder *create(const tyrsound_Format& fmt, tyrsound_Stream strm);
    static const char *getName() { return "FLAC"; }
    static void staticInit() {}
    static void staticShutdown() {}
    static bool checkMagic(const unsigned char *magic, size_t size);

    virtual size_t fillBuffer(void *buf, size_t size);
    virtual float getLength();
    virtual tyrsound_Error seek(float seconds);
    virtual float tell();
    virtual tyrsound_Error setLoop(float seconds, int loops);
    virtual float getLoopPoint();
    virtual bool isEOF();
    virtual void getFormat(tyrsound_Format *fmt);

    int writeCallback(const void *decoder, const void *frame, const void *const buffer[]);

private:

    void *_samplebuf;

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
