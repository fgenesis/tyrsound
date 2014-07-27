#include "tyrsound_internal.h"
#include "NullChannel.h"
#include "NullDevice.h"

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DEVICE("null", NullDevice);


NullDevice::NullDevice()
: _channels(NULL)
{
}

NullDevice::~NullDevice()
{
    for(unsigned int i = 0; i < _numChannels; ++i)
        if(ChannelBase *chan = _channels[i])
            chan->destroy();
    Free(_channels);
}

NullDevice *NullDevice::create(tyrsound_Format& fmt, tyrsound_DeviceConfig& cfg)
{
    if(!cfg.playbackChannels)
        cfg.playbackChannels = 16;
    if(!cfg.bufferSize)
        cfg.bufferSize = 1024 * 16;
    if(!fmt.channels)
        fmt.channels = 2;
    if(!fmt.hz)
        fmt.hz = 44100;

    void *mem = Alloc(sizeof(NullDevice));
    if(!mem)
        return NULL;

    NullDevice *dev = new(mem) NullDevice();

    dev->_fmt = fmt;
    dev->_cfg = cfg;

    if(!dev->_allocateChannels(fmt.channels))
    {
        dev->destroy();
        return NULL;
    }

    return dev;
}

bool NullDevice::_allocateChannels(unsigned int num)
{
    size_t bytes = num * sizeof(NullChannel*);
    _channels = (NullChannel**)Alloc(bytes);
    memset(_channels, 0, bytes);
    if(!_channels)
        return false;

    _numChannels = num;
    return true;
}

tyrsound_Error NullDevice::setVolume(float vol)
{
    return TYRSOUND_ERR_OK;
}

void NullDevice::update()
{
    MutexGuard guard(_channelLock);

    for(unsigned int i = 0; i < _numChannels; ++i)
        if(_channels[i]->x_acquired)
            _channels[i]->update();
}

ChannelBase *NullDevice::reserveChannel()
{
    MutexGuard guard(_channelLock);
    if(!guard)
        return NULL;

    int freeSlot = -1;
    for(unsigned int i = 0; i < _numChannels; ++i)
        if(!_channels[i])
            freeSlot = i;

    if(freeSlot >= 0)
    {
        void *mem = Alloc(sizeof(NullChannel));
        if(!mem)
            return NULL;
        NullChannel *chan = NullChannel::create(this, _fmt);
        _channels[freeSlot] = chan;
        return chan;
    }

    return NULL;
}

void NullDevice::retainChannel(ChannelBase *chan)
{
    {
        MutexGuard guard(_channelLock);
        if(!guard)
            breakpoint();

        for(unsigned int i = 0; i < _numChannels; ++i)
            if(_channels[i] == chan)
            {
                _channels[i] = NULL;
                break;
            }
    }
    chan->destroy();
}

void NullDevice::acquireChannel(ChannelBase *chan)
{
    MutexGuard guard(_channelLock);
    if(!guard)
        breakpoint();

    for(unsigned int i = 0; i < _numChannels; ++i)
        if(_channels[i] == chan)
        {
            _channels[i]->x_acquired = false;
            _channels[i] = NULL;
            break;
        }
}

tyrsound_Error NullDevice::setSpeed(float speed)
{
    return TYRSOUND_ERR_OK;
}

tyrsound_Error NullDevice::setPosition(float x, float y, float z)
{
    return TYRSOUND_ERR_OK;
}

#include "tyrsound_end.h"
