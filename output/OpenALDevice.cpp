#include "OpenALDevice.h"
#include "tyrsound_internal.h"

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DEVICE("openal", OpenALDevice);


OpenALDevice::OpenALDevice()
{
}

OpenALDevice::~OpenALDevice()
{
}

OpenALDevice *OpenALDevice::create(tyrsound_Format& fmt)
{
    void *mem = Alloc(sizeof(OpenALDevice));
    if(!mem)
        return NULL;

    OpenALDevice *dev = new(mem) OpenALDevice();

    return dev;
}

bool OpenALDevice::_allocateChannels(unsigned int num)
{
    return false;
}

tyrsound_Error OpenALDevice::setVolume(float vol)
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

void OpenALDevice::update()
{
}

ChannelBase *OpenALDevice::getFreeChannel()
{
    return NULL;
}


#include "tyrsound_end.h"
