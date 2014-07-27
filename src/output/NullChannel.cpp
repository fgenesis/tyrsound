#include "tyrsound_internal.h"
#include "NullChannel.h"
#include "NullDevice.h"

#include "tyrsound_begin.h"


NullChannel::NullChannel()
: _buffer(NULL), _playing(false), _wantData(true), x_acquired(false)
{
}

NullChannel::~NullChannel()
{
    Free(_buffer);
}

NullChannel *NullChannel::create(NullDevice *dev, const tyrsound_Format& fmt)
{
    size_t bufsize = dev->_cfg.bufferSize;

    void *mem = Alloc(sizeof(NullChannel));
    if(!mem)
        return NULL;
    NullChannel *chan = new(mem) NullChannel();

    mem = Alloc(bufsize);
    if(!mem)
    {
        chan->destroy();
        return NULL;
    }

    chan->_bufsize = bufsize;
    chan->_buffer = mem;

    return chan;
}

bool NullChannel::wantData()
{
    return _wantData;
}

void NullChannel::getBuffer(void **buf, size_t *size)
{
    *buf = _buffer;
    *size = _bufsize;
}

void NullChannel::update()
{
    _wantData = true;
}

tyrsound_Error NullChannel::filledBuffer(size_t size, const tyrsound_Format& fmt)
{
    _wantData = false;
    if(!size) // eof?
        _playing = false;
    _samplesDone += size;
    _hz = fmt.hz;
    return TYRSOUND_ERR_OK;
}

tyrsound_Error NullChannel::setVolume(float vol)
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

tyrsound_Error NullChannel::setSpeed(float vol)
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

tyrsound_Error NullChannel::pause()
{
    _playing = false;
    return TYRSOUND_ERR_OK;
}

tyrsound_Error NullChannel::play()
{
    _playing = true;
    return TYRSOUND_ERR_OK;
}

tyrsound_Error NullChannel::stop()
{
    _playing = false;
    _samplesDone = 0;
    return TYRSOUND_ERR_OK;
}

bool NullChannel::isPlaying()
{
    return _playing;
}

float NullChannel::getPlayPosition()
{
    return (_samplesDone / _hz) + (float(_samplesDone % _hz) / float(_hz));
}

tyrsound_Error NullChannel::setPosition(float x, float y, float z)
{
    return TYRSOUND_ERR_OK;
}

#include "tyrsound_end.h"
