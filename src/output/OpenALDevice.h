#ifndef TYRSOUND_OPENAL_DEVICE_H
#define TYRSOUND_OPENAL_DEVICE_H

#include "DeviceBase.h"

#include "tyrsound_begin.h"

class OpenALDevice : public DeviceBase
{
protected:
    OpenALDevice();
    virtual ~OpenALDevice();

    bool _allocateChannels(unsigned int num);

public:
    static OpenALDevice *create(tyrsound_Format& fmt);

    virtual tyrsound_Error setVolume(float vol);
    virtual void update();
    virtual ChannelBase *getFreeChannel();
};


#include "tyrsound_end.h"

#endif

