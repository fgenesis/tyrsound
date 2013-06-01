#include "tyrsound.h"
#include "tyrsound_internal.h"

// Output devices
#include "NullDevice.h"
#include "OpenALDevice.h"



#include "tyrsound_begin.h"

static tyrsound_Format s_format;
static DeviceBase *s_device = NULL;


static void _applyDefaultFormat(tyrsound_Format& fmt)
{
    fmt.sampleBits = 16;
    fmt.bufferSize = 16 * 1024;
    fmt.channels = 0;
    fmt.hz = 44100;
    fmt.numBuffers = 8;
    fmt.bigendian = 0;
    fmt.signedSamples = 1;
}

DeviceBase *getDevice()
{
    return s_device;
}

void shutdownDevice()
{
    if(s_device)
    {
        s_device->destroy();
        s_device = NULL;
        memset(&s_format, 0, sizeof(s_format));
    }
}

bool initDevice(const char *name, const tyrsound_Format *fmt)
{
    if(fmt)
        s_format = *fmt;
    else
        _applyDefaultFormat(s_format);

    for(size_t i = 0; i < TYRSOUND_DEVICE_HOLDER::Size(); ++i)
    {
        const DeviceInfo& di = TYRSOUND_DEVICE_HOLDER::Get(i);
        // allow all devices except null device when name is not specified
        if((name && !strcmp(name, di.name)) || (!name && strcmp(di.name, "null")))
        {
            s_device = di.factory->create(s_format);
            if(s_device)
                return true;
            if(name) // when requesting a device by name and it failed, bail out
                return false;
        }
    }

    return false;
}


#include "tyrsound_end.h"


void tyrsound_getFormat(tyrsound_Format *fmt)
{
    *fmt = tyrsound::s_format;
}

tyrsound_Error tyrsound_setListenerPosition(float x, float y, float z)
{
    return tyrsound::s_device->setPosition(x, y, z);
}

tyrsound_Error tyrsound_setMasterVolume(float vol)
{
    return tyrsound::s_device->setVolume(vol);
}

tyrsound_Error tyrsound_setMasterSpeed(float speed)
{
    return tyrsound::s_device->setSpeed(speed);
}
