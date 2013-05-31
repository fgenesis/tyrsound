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
    fmt.bufferSize = 8 * 1024;
    fmt.channels = 0;
    fmt.hz = 44100;
    fmt.numBuffers = 8;
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
        if(!strcmp(name, di.name))
            return (s_device = di.factory->create(s_format)) != NULL;
    }

    return false;
}


#include "tyrsound_end.h"


void tyrsound_getFormat(tyrsound_Format *fmt)
{
    *fmt = tyrsound::s_format;
}
