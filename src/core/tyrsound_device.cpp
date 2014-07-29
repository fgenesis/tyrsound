#include "tyrsound_internal.h"

// Output devices
#include "NullDevice.h"
#include "OpenALDevice.h"

#include "tyrsound_begin.h"

#define DEVICE_HOLDER tyrsound::RegistrationHolder<tyrsound::DeviceInfo, 32>

static tyrsound_Format s_format;
static DeviceBase *s_device = NULL;


static void _applyDefaultFormat(tyrsound_Format& fmt)
{
    fmt.sampleBits = 16;
    fmt.channels = 2;
    fmt.hz = 44100;
    fmt.bigendian = isBigEndian();
    fmt.signedSamples = 1;
    fmt.isfloat = 0;
}


static void _fixFormat(tyrsound_Format& fmt)
{
    if(fmt.isfloat)
    {
        fmt.sampleBits = sizeof(float) * 8;
        fmt.signedSamples = 1; // irrelevant for float
    }
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

bool initDevice(const char *name, const tyrsound_Format *fmt, const tyrsound_DeviceConfig *cfg)
{
    if(fmt)
        s_format = *fmt;
    else
        _applyDefaultFormat(s_format);

    tyrsound_DeviceConfig localCfg;
    if(!cfg)
    {
        memset(&localCfg, 0, sizeof(*cfg));
        cfg = &localCfg;
    }

    const unsigned int numDevices = DEVICE_HOLDER::Size();
    for(size_t i = 0; i < numDevices; ++i)
    {
        const DeviceInfo& di = DEVICE_HOLDER::Get(i);
        // allow all devices except null device when name is not specified
        if((name && !strcmp(name, di.name)) || (!name && strcmp(di.name, "null")))
        {
            tyrsound_Format usefmt = s_format;
            tyrsound_DeviceConfig usecfg = *cfg;
            s_device = di.factory->create(usefmt, usecfg); // may change format and then fail, so work on a copy
            if(s_device)
            {
                s_format = usefmt;
                _fixFormat(s_format);
                tyrsound_ex_messagef(TYRSOUND_MSG_INFO, "Using device: %s", di.name);
                tyrsound_ex_messagef(TYRSOUND_MSG_INFO, "  Format: rate=%d, bits=%d, channels=%d, bigendian=%d, signed=%d, float=%d",
                    s_format.hz, s_format.sampleBits, s_format.channels, s_format.bigendian, s_format.signedSamples, s_format.isfloat);
                tyrsound_ex_messagef(TYRSOUND_MSG_INFO, "  Config: bufsize=%d, nbuf=%d, playbackChannels=%d",
                    usecfg.bufferSize, usecfg.numBuffers, usecfg.playbackChannels);
                return true;
            }
            if(name) // when requesting a device by name and it failed, bail out
                return false;
        }
    }

    return false;
}


#include "tyrsound_end.h"



TYRSOUND_DLL_EXPORT void tyrsound_ex_registerDevice(const tyrsound::DeviceInfo& di)
{
    DEVICE_HOLDER::Register(di);
}

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
