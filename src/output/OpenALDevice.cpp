#include <al.h>
#include <alc.h>

#include "OpenALDevice.h"
#include "OpenALChannel.h"
#include "tyrsound_internal.h"

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DEVICE("openal", OpenALDevice);


static bool adjustFormat(ALCdevice *dev, tyrsound_Format& fmt)
{
    bool good = true;
    ALCint n = 0;
    alcGetIntegerv(dev, ALC_ATTRIBUTES_SIZE, 1, &n);
    bool hasMono = false, hasStereo = false;
    if(n)
    {
        ALCint *attr = (ALint*)Alloc(n * sizeof(ALint));
        alcGetIntegerv(dev, ALC_ALL_ATTRIBUTES, n, attr);
        for(ALCint i = 0; i < n; i += 2)
        {
            switch(attr[i])
            {
                case ALC_MONO_SOURCES:
                     hasMono = true;
                     // fallthrough
                case ALC_STEREO_SOURCES:
                    hasStereo = true;
                    good = good && attr[i] > 0;
                    fmt.channels = Min<unsigned int>(fmt.channels, attr[i]);
                    break;
            }
        }
        Free(attr);
    }

    return good && hasMono && hasStereo;
}


OpenALDevice::OpenALDevice(const tyrsound_Format& fmt,  void *dev, void *ctx)
: _fmt(fmt)
, _dev(dev)
, _ctx(ctx)
, _channels(NULL)
, _channelsAllocated(0)
{
}

#define ALCTX ((ALCcontext*)_ctx)
#define ALDEV ((ALCdevice*)_dev)

OpenALDevice::~OpenALDevice()
{
    if(_channels)
    {
        for(unsigned int i = 0; i < _channelsAllocated; ++i)
            _channels[i].~OpenALChannel();
        Free(_channels);
    }

    alcMakeContextCurrent(NULL);
    alcSuspendContext(ALCTX);
    alcDestroyContext(ALCTX);
    alcCloseDevice(ALDEV);
}

OpenALDevice *OpenALDevice::create(tyrsound_Format& fmt)
{
    const unsigned int channels = fmt.channels ? fmt.channels : 64;
    fmt.channels = channels;

    ALCdevice *dev = alcOpenDevice(NULL); // TODO: allow specifying this in detail
    if (!dev)
        return NULL;

    ALCint req_attributes[] =
    {
        ALC_MONO_SOURCES,
        channels,
        ALC_STEREO_SOURCES,
        channels,
        0
    };

    ALCcontext *ctx = alcCreateContext(dev, req_attributes);
    if (!ctx)
    {
        alcCloseDevice(dev);
        return NULL;
    }

    void *mem = Alloc(sizeof(OpenALDevice));

    if(!mem || !adjustFormat(dev, fmt))
    {
        alcDestroyContext(ctx);
        alcCloseDevice(dev);
        return NULL;
    }

    alcMakeContextCurrent(ctx);
    alcProcessContext(ctx);

    OpenALDevice *aldev = new(mem) OpenALDevice(fmt, dev, ctx);

    if(!aldev->_allocateChannels())
    {
        aldev->destroy();
        return NULL;
    }

    fmt = aldev->_fmt; // might have updated something in the class while setting up

    return aldev;
}

bool OpenALDevice::_allocateChannels()
{
    unsigned int totalchannels = _fmt.channels;
    _channels = (OpenALChannel*)Alloc(sizeof(OpenALChannel) * totalchannels);
    if(!_channels)
        return false;
    _channelsAllocated = totalchannels;
    for(unsigned int i = 0; i < totalchannels; ++i)
        new ((void*)&_channels[i]) OpenALChannel(this); // nasty in-place construction

    ALenum err = alGetError();

    for (unsigned int i = 0; i < totalchannels; i++)
    {
        ALuint sid = 0;
        alGenSources(1, &sid);

        // Workaround for AL bug that it would report more available channels than it really can provide.
        err = alGetError();
        if (err != AL_NO_ERROR || !sid)
        {
            _fmt.channels = i;
            break;
        }
        _channels[i].setSourceid(sid);
    }
    return _fmt.channels > 0;
}

void OpenALDevice::update()
{
}

ChannelBase *OpenALDevice::getFreeChannel()
{
    for(unsigned int i = 0; i < _fmt.channels; ++i)
    {
        OpenALChannel &chan = _channels[i];
        if(chan.isFree())
            return &chan;
    }
    return NULL;
}

tyrsound_Error OpenALDevice::setSpeed(float speed)
{
    alListenerf(AL_PITCH, speed);
    return alGetError() == AL_NO_ERROR ? TYRSOUND_ERR_OK : TYRSOUND_ERR_UNSPECIFIED;
}

tyrsound_Error OpenALDevice::setVolume(float vol)
{
    alListenerf(AL_GAIN, vol);
    return alGetError() == AL_NO_ERROR ? TYRSOUND_ERR_OK : TYRSOUND_ERR_UNSPECIFIED;
}

tyrsound_Error OpenALDevice::setPosition(float x, float y, float z)
{
    alListener3f(AL_POSITION, x, y, z);
    return alGetError() == AL_NO_ERROR ? TYRSOUND_ERR_OK : TYRSOUND_ERR_UNSPECIFIED;
}


#include "tyrsound_end.h"
