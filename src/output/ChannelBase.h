#ifndef TYRSOUND_OUTPUT_BASE_H
#define TYRSOUND_OUTPUT_BASE_H
#include "tyrsound.h"
#include "tyrsound_begin.h"


class ChannelBase
{
protected:

    ChannelBase() {}
    virtual ~ChannelBase() {}

public:
    virtual tyrsound_Error setVolume(float vol) = 0;
    virtual tyrsound_Error setSpeed(float speed) = 0;
    virtual tyrsound_Error stop() = 0;
    virtual tyrsound_Error play() = 0;
    virtual tyrsound_Error pause() = 0;
    virtual bool isPlaying() = 0;

    virtual bool wantData() = 0;
    virtual void getBuffer(void **buf, size_t *size) = 0;
    virtual void update() = 0;
    virtual void filledBuffer(size_t size) = 0;
    virtual bool isFree() = 0;



    void destroy();
};


#include "tyrsound_end.h"
#endif
