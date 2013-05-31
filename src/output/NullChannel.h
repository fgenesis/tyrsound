#ifndef TYRSOUND_NULL_OUTPUT_H
#define TYRSOUND_NULL_OUTPUT_H

#include "ChannelBase.h"

#include "tyrsound_begin.h"


class NullChannel : public ChannelBase
{
protected:
    NullChannel();
    ~NullChannel();

public:
    static NullChannel *create(const tyrsound_Format& fmt);

    virtual tyrsound_Error setVolume(float vol);
    virtual tyrsound_Error setSpeed(float speed);
    virtual tyrsound_Error stop();
    virtual tyrsound_Error play();
    virtual tyrsound_Error pause();
    virtual bool isPlaying();

    virtual bool wantData();
    virtual void getBuffer(void **buf, size_t *size);
    virtual void update();
    virtual void filledBuffer(size_t size);
    virtual bool isFree();

protected:
    void *_buffer;
    size_t _bufsize;
    bool _playing;
    bool _inUse;
    bool _wantData;
};

#include "tyrsound_end.h"
#endif
