#include <al.h>
#include <alc.h>
#ifdef TYRSOUND_USE_ALEXT
#  include <alext.h>
#endif

#include "tyrsound_internal.h"
#include "OpenALDevice.h"
#include "OpenALChannel.h"

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DEVICE("openal", OpenALDevice);

#ifndef AL_FORMAT_MONO_FLOAT32
  static int AL_FORMAT_MONO_FLOAT32 = 0;
#endif
#ifndef AL_FORMAT_STEREO_FLOAT32
  static int AL_FORMAT_STEREO_FLOAT32 = 0;
#endif
#ifndef AL_EXT_float32
#  define AL_EXT_float32 0
#endif


ALenum OpenALDevice::getALFormat(const tyrsound_Format& fmt, unsigned int *bytesPerSample)
{
    ALint alformat = -1;
    unsigned int bps = 0;
    switch(fmt.channels)
    {
    case 1:
        if(AL_FORMAT_STEREO_FLOAT32 && fmt.isfloat)
        {
            alformat = AL_FORMAT_MONO_FLOAT32;
            bps = sizeof(float);
        }
        else if(fmt.sampleBits <= 8)
        {
            alformat = AL_FORMAT_MONO8;
            bps = 1;
        }
        else if(fmt.sampleBits <= 16)
        {
            alformat = AL_FORMAT_MONO16;
            bps = 2;
        }
        break;

    case 2:
        if(AL_FORMAT_STEREO_FLOAT32 && fmt.isfloat)
        {
            alformat = AL_FORMAT_STEREO_FLOAT32;
            bps = 2 * sizeof(float);
        }
        else if(fmt.sampleBits <= 8)
        {
            alformat = AL_FORMAT_STEREO8;
            bps = 2;
        }
        else
        {
            alformat = AL_FORMAT_STEREO16;
            bps = 4;
        }
        break;
    }
    if(bytesPerSample)
        *bytesPerSample = bps;
    return alformat;
}


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


OpenALDevice::OpenALDevice(const tyrsound_Format& fmt, const tyrsound_DeviceConfig& cfg, void *dev, void *ctx)
: _fmt(fmt)
, _cfg(cfg)
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
    if(_chStatus)
        Free(_chStatus);

    alcMakeContextCurrent(NULL);
    alcSuspendContext(ALCTX);
    alcDestroyContext(ALCTX);
    alcCloseDevice(ALDEV);
}

OpenALDevice *OpenALDevice::create(tyrsound_Format& fmt, tyrsound_DeviceConfig& cfg)
{
    if(!fmt.channels)
        fmt.channels = 2;
    if(!fmt.hz)
        fmt.hz = 44100;
    if(!cfg.playbackChannels)
        cfg.playbackChannels = 64;

    if(!cfg.bufferSize)
        cfg.bufferSize = 1024 * 16;
    if(!cfg.numBuffers)
        cfg.numBuffers = 8;

    // See http://digitalstarcode.googlecode.com/svn-history/r39/trunk/ofxSoundPlayerExample/src/ofxSoundPlayer/OpenAL/AudioDevice.cpp

    ALCdevice *dev = alcOpenDevice(NULL); // TODO: allow specifying this in detail
    if (!dev)
        return NULL;

    ALCint req_attributes[] =
    {
        ALC_MONO_SOURCES,
        cfg.playbackChannels,
        ALC_STEREO_SOURCES,
        cfg.playbackChannels,
        0
    };

    ALCcontext *ctx = alcCreateContext(dev, req_attributes);
    if (!ctx)
    {
        alcCloseDevice(dev);
        return NULL;
    }

    // This is a bit messy. OpenALSoft pretends not to know AL_EXT_float32, even though it does.
    // But it will return values for AL_FORMAT_[MONO|STEREO]_FLOAT32.
    // So even in case defines exist in the headers, the capabilities of the actual system implementation are unknown.
    // So we can assume that if the enum value is known, float is supported.
    bool hasFloat = false;
    if(AL_EXT_float32 || alIsExtensionPresent("AL_EXT_float32") == AL_TRUE)
    {
        hasFloat = true;

        ALenum val = alcGetEnumValue(dev, "AL_FORMAT_MONO_FLOAT32");
        hasFloat = hasFloat && val;
        #ifndef AL_FORMAT_MONO_FLOAT32
        AL_FORMAT_MONO_FLOAT32 = val;
        #endif

        val = alcGetEnumValue(dev, "AL_FORMAT_STEREO_FLOAT32");
        hasFloat = hasFloat && val;
        #ifndef AL_FORMAT_STEREO_FLOAT32
        AL_FORMAT_STEREO_FLOAT32 = val;
        #endif

        alGetError(); // drop errors
    }

    fmt.isfloat = hasFloat && fmt.isfloat && AL_FORMAT_MONO_FLOAT32 && AL_FORMAT_STEREO_FLOAT32;

    tyrsound_ex_messagef(TYRSOUND_MSG_INFO, "OpenAL extension AL_EXT_float32 is %s", hasFloat ? "supported" : "NOT supported");

    void *mem = Alloc(sizeof(OpenALDevice));

    if(!mem || !adjustFormat(dev, fmt))
    {
        alcDestroyContext(ctx);
        alcCloseDevice(dev);
        return NULL;
    }

    alcMakeContextCurrent(ctx);
    alcProcessContext(ctx);

    OpenALDevice *aldev = new(mem) OpenALDevice(fmt, cfg, dev, ctx);

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
    const unsigned int totalchannels = _fmt.channels;
    _channels = (OpenALChannel*)Alloc(sizeof(OpenALChannel) * totalchannels);
    if(!_channels)
        return false;
    _chStatus = (ChannelStatus*)Alloc(sizeof(ChannelStatus) * totalchannels);
    if(!_chStatus)
        return false;
    _channelsAllocated = totalchannels;
    for(unsigned int i = 0; i < totalchannels; ++i)
    {
        new ((void*)&_channels[i]) OpenALChannel(this); // nasty in-place construction
        _channels[i].x_channelIndex = i;
        _chStatus[i] = ALCHAN_FREE;
    }

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
    MutexGuard guard(_channelLock);

    for(unsigned int i = 0; i < _fmt.channels; ++i)
        if(_chStatus[i] == ALCHAN_INUSE)
            _channels[i].update();
}

ChannelBase *OpenALDevice::reserveChannel()
{
    MutexGuard guard(_channelLock);
    if(!guard)
        return NULL;

    for(unsigned int i = 0; i < _fmt.channels; ++i)
    {
        if(_chStatus[i] == ALCHAN_FREE)
        {
            _chStatus[i] = ALCHAN_RESERVED;
            return &_channels[i];
        }
    }
    return NULL;
}

void OpenALDevice::acquireChannel(ChannelBase *chan)
{
    MutexGuard guard(_channelLock);
    if(!guard)
        breakpoint();

    OpenALChannel *alchan = (OpenALChannel*)chan;
    _chStatus[alchan->x_channelIndex] = ALCHAN_INUSE;
}

void OpenALDevice::retainChannel(ChannelBase *chan)
{
    MutexGuard guard(_channelLock);
    if(!guard)
        breakpoint();

    OpenALChannel *alchan = (OpenALChannel*)chan;
    _chStatus[alchan->x_channelIndex] = ALCHAN_FREE;
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
