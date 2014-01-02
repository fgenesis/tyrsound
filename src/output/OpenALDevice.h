#ifndef TYRSOUND_OPENAL_DEVICE_H
#define TYRSOUND_OPENAL_DEVICE_H

#include "tyrDeviceBase.h"

#include "tyrsound_begin.h"

class OpenALChannel;

class OpenALDevice : public DeviceBase
{
    friend class OpenALChannel;

protected:
    OpenALDevice(const tyrsound_Format& fmt, void *dev, void *ctx);
    virtual ~OpenALDevice();

    bool _allocateChannels();

public:
    static OpenALDevice *create(tyrsound_Format& fmt);

    virtual tyrsound_Error setVolume(float vol);
    virtual tyrsound_Error setSpeed(float speed);
    virtual tyrsound_Error setPosition(float x, float y, float z);
    virtual void update();
    virtual ChannelBase *reserveChannel(); // return a free channel, or NULL if no free channel could be found
    virtual void retainChannel(ChannelBase *); // once a channel is done, this is used so that it can be reserveChannel()'d again.

private:
    void *_dev; // ALCdevice
    void *_ctx; // ALCcontext
    tyrsound_Format _fmt;
    OpenALChannel *_channels;
    bool *_reserved;
    unsigned int _channelsAllocated;
    Mutex _channelLock;
};


#include "tyrsound_end.h"

#endif

