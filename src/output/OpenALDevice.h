#ifndef TYRSOUND_OPENAL_DEVICE_H
#define TYRSOUND_OPENAL_DEVICE_H

#include "tyrDeviceBase.h"

#include "tyrsound_begin.h"

class OpenALChannel;

class OpenALDevice : public DeviceBase
{
    friend class OpenALChannel;

    enum ChannelStatus
    {
        ALCHAN_FREE,
        ALCHAN_RESERVED,
        ALCHAN_INUSE
    };

protected:
    OpenALDevice(const tyrsound_Format& fmt, const tyrsound_DeviceConfig& cfg, void *dev, void *ctx);
    virtual ~OpenALDevice();

    bool _allocateChannels();

public:
    static OpenALDevice *create(tyrsound_Format& fmt, tyrsound_DeviceConfig& cfg);

    virtual tyrsound_Error setVolume(float vol);
    virtual tyrsound_Error setSpeed(float speed);
    virtual tyrsound_Error setPosition(float x, float y, float z);
    virtual void update();
    virtual ChannelBase *reserveChannel();
    virtual void acquireChannel(ChannelBase *);
    virtual void retainChannel(ChannelBase *);

private:
    // Used by OpenALChannel
    static int getALFormat(const tyrsound_Format& fmt, unsigned int *bytesPerSample); // ALEnum

    void *_dev; // ALCdevice
    void *_ctx; // ALCcontext
    tyrsound_Format _fmt;
    tyrsound_DeviceConfig _cfg;
    OpenALChannel *_channels;
    ChannelStatus *_chStatus;
    unsigned int _channelsAllocated;
    Mutex _channelLock;
};


#include "tyrsound_end.h"

#endif

