#ifndef TYRSOUND_NULL_CHANNEL_H
#define TYRSOUND_NULL_CHANNEL_H

#include "tyrChannelBase.h"

#include "tyrsound_begin.h"


class NullDevice;

class NullChannel : public ChannelBase
{
    friend class NullDevice;

protected:
    NullChannel();
    ~NullChannel();

public:
    static NullChannel *create(NullDevice *dev, const tyrsound_Format& fmt);

    virtual tyrsound_Error prepare() { return TYRSOUND_ERR_OK; }
    virtual tyrsound_Error setVolume(float vol);
    virtual tyrsound_Error setSpeed(float speed);
    virtual tyrsound_Error stop();
    virtual tyrsound_Error play();
    virtual tyrsound_Error pause();
    virtual tyrsound_Error setPosition(float x, float y, float z);
    virtual bool isPlaying();
    virtual float getPlayPosition();

    virtual bool wantData();
    virtual void getBuffer(void **buf, size_t *size);
    virtual void update();
    virtual tyrsound_Error filledBuffer(size_t size, const tyrsound_Format& fmt);

protected:
    void *_buffer;
    NullDevice *_dev;
    size_t _bufsize;
    bool _playing;
    bool _wantData;
    tyrsound_int64 _samplesDone;
    int _hz;

private:
    bool x_acquired;
};

#include "tyrsound_end.h"
#endif
