#include "NullDevice.h"
#include "NullChannel.h"
#include "tyrsound_internal.h"

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

NullDevice *NullDevice::create(tyrsound_Format& fmt)
{
    void *mem = Alloc(sizeof(NullDevice));
    if(!mem)
        return NULL;

    NullDevice *dev = new(mem) NullDevice();

    dev->_fmt = fmt;

    if(!dev->_allocateChannels(fmt.channels ? fmt.channels : 16))
    {
        dev->destroy();
        return NULL;
    }

    return dev;
}

bool NullDevice::_allocateChannels(unsigned int num)
{
    size_t bytes = num * sizeof(ChannelBase*);
    _channels = (ChannelBase**)Alloc(bytes);
    memset(_channels, 0, bytes);
    if(!_channels)
        return false;

    _numChannels = num;
    return true;
}

tyrsound_Error NullDevice::setVolume(float vol)
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

void NullDevice::update()
{
    for(unsigned int i = 0; i < _numChannels; ++i)
        if(ChannelBase *chan = _channels[i])
            chan->update();
}

ChannelBase *NullDevice::getFreeChannel()
{
    int freeSlot = -1;
    for(unsigned int i = 0; i < _numChannels; ++i)
    {
        ChannelBase *chan = _channels[i];
        if(!chan)
            freeSlot = i;
        else if(chan->isFree())
            return chan;
    }

    if(freeSlot >= 0)
    {
        void *mem = Alloc(sizeof(NullChannel));
        if(!mem)
            return NULL;
        ChannelBase *chan = NullChannel::create(_fmt);
        _channels[freeSlot] = chan;
        return chan;
    }

    return NULL;
}


#include "tyrsound_end.h"
