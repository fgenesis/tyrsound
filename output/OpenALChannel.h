#ifndef TYRSOUND_OPENAL_OUTPUT_H
#define TYRSOUND_OPENAL_OUTPUT_H

#include "ChannelBase.h"

#include "tyrsound_begin.h"


class OpenALChannel : public ChannelBase
{
protected:
    OpenALChannel();
    virtual ~OpenALChannel();

public:
    static OpenALChannel *create(const tyrsound_Format& fmt);

    virtual tyrsound_Error setVolume(float vol);
    virtual tyrsound_Error setSpeed(float speed);
    virtual tyrsound_Error stop();
    virtual tyrsound_Error play();
    virtual tyrsound_Error pause();
    virtual bool isPlaying() const;

    virtual bool wantData();
    virtual void getBuffer(void **buf, size_t *size);
    virtual void update();
    virtual void filledBuffer(size_t size);

protected:
    //void *_buffer;
};



#include "tyrsound_end.h"

#endif
