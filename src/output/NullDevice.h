#ifndef TYRSOUND_NULL_DEVICE_H
#define TYRSOUND_NULL_DEVICE_H

#include "tyrDeviceBase.h"

#include "tyrsound_begin.h"

class NullDevice : public DeviceBase
{
protected:
    NullDevice();
    virtual ~NullDevice();

    bool _allocateChannels(unsigned int num);

public:
    static NullDevice *create(tyrsound_Format& fmt);

    virtual tyrsound_Error setVolume(float vol);
    virtual tyrsound_Error setSpeed(float speed);
    virtual tyrsound_Error setPosition(float x, float y, float z);
    virtual void update();
    virtual ChannelBase *getFreeChannel();

private:
    ChannelBase **_channels;
    unsigned int _numChannels;
    tyrsound_Format _fmt;
};


#include "tyrsound_end.h"

#endif

