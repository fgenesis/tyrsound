#ifndef TYRSOUND_OGG_DECODER_H
#define TYRSOUND_OGG_DECODER_H
#include "tyrsound_internal.h"

#include "DecoderBase.h"

#include "tyrsound_begin.h"


class OggDecoder : public DecoderBase
{
protected:
    OggDecoder(void *state, const tyrsound_Format& fmt);
    virtual ~OggDecoder();

public:
    static OggDecoder *create(const tyrsound_Format& fmt, tyrsound_Stream strm);

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
    unsigned int _sampleWordSize;
    float _loopPoint;
    int _loopCount;
    bool _seekable;
    bool _eof;
};



#include "tyrsound_end.h"
#endif
