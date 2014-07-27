#include "tyrsound_internal.h"

// Output devices
#include "NullDevice.h"
#include "OpenALDevice.h"

#include "tyrsound_begin.h"

#define TYRSOUND_DEVICE_HOLDER RegistrationHolder<DeviceInfo, 32>

static tyrsound_Format s_format;
static DeviceBase *s_device = NULL;


static void _applyDefaultFormat(tyrsound_Format& fmt)
{
    fmt.sampleBits = 16;
    fmt.bufferSize = 16 * 1024;
    fmt.channels = 0;
    fmt.hz = 44100;
    fmt.numBuffers = 8;
    fmt.bigendian = isBigEndian();
    fmt.signedSamples = 1;
    fmt.isfloat = 0; // -1: use float if possible
}

TYRSOUND_DLL_EXPORT void tyrsound_ex_registerDevice(const DeviceInfo& di)
{
    TYRSOUND_DEVICE_HOLDER::Register(di);
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

    const unsigned int numDevices = TYRSOUND_DEVICE_HOLDER::Size();
    for(size_t i = 0; i < numDevices; ++i)
    {
        const DeviceInfo& di = TYRSOUND_DEVICE_HOLDER::Get(i);
        // allow all devices except null device when name is not specified
        if((name && !strcmp(name, di.name)) || (!name && strcmp(di.name, "null")))
        {
            s_device = di.factory->create(s_format);
            if(s_device)
            {
                tyrsound_ex_messagef(TYRSOUND_MSG_INFO, "Using device: %s", di.name);
                return true;
            }
            if(name) // when requesting a device by name and it failed, bail out
                return false;
        }
    }

    return false;
}


#include "tyrsound_end.h"


TYRSOUND_DLL_EXPORT void tyrsound_getFormat(tyrsound_Format *fmt)
{
    *fmt = tyrsound::s_format;
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setListenerPosition(float x, float y, float z)
{
    return tyrsound::s_device->setPosition(x, y, z);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setMasterVolume(float vol)
{
    return tyrsound::s_device->setVolume(vol);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_setMasterSpeed(float speed)
{
    return tyrsound::s_device->setSpeed(speed);
}
