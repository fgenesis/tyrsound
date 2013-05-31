#include "NullChannel.h"
#include "tyrsound_internal.h"
#include "tyrsound_begin.h"


NullChannel::NullChannel()
: _buffer(NULL), _playing(false), _inUse(false), _wantData(true)
{
}

NullChannel::~NullChannel()
{
    Free(_buffer);
}

NullChannel *NullChannel::create(const tyrsound_Format& fmt)
{
    size_t bufsize = fmt.bufferSize;

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

void NullChannel::filledBuffer(size_t size)
{
    _wantData = false;
    if(!size) // eof?
        _playing = false;
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
    _inUse = true;
    return TYRSOUND_ERR_OK;
}

tyrsound_Error NullChannel::stop()
{
    _playing = false;
    _inUse = false;
    return TYRSOUND_ERR_OK;
}

bool NullChannel::isPlaying()
{
    return _playing;
}

bool NullChannel::isFree()
{
    return !_inUse;
}

#include "tyrsound_end.h"
