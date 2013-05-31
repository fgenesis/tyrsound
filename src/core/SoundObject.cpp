#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "DecoderBase.h"
#include "ChannelBase.h"

#include "tyrsound_begin.h"

SoundObject *SoundObject::create(DecoderBase *decoder, ChannelBase *output)
{
    void *p = Alloc(sizeof(SoundObject));
    return new(p) SoundObject(decoder, output);
}

void SoundObject::destroy()
{
    stop();
    _decoder->destroy();

    this->~SoundObject();
    Free(this);
}

SoundObject::~SoundObject()
{
}

SoundObject::SoundObject(DecoderBase *decoder, ChannelBase *channel)
: _idxInStore(unsigned(-1))
, _decoder(decoder)
, _channel(channel)
{
}

void SoundObject::update()
{
    //_channel->update();
    while(_channel->wantData())
    {
        void *buf = NULL;
        size_t size = 0;
        _channel->getBuffer(&buf, &size);
        if(buf && size)
        {
            size_t filled = _decoder->fillBuffer(buf, size);
            _channel->filledBuffer(filled);
        }
    }
}

tyrsound_Error SoundObject::setVolume(float vol)
{
    return _channel->setVolume(vol);
}

tyrsound_Error SoundObject::setSpeed(float speed)
{
    return _channel->setSpeed(speed);
}

tyrsound_Error SoundObject::play()
{
    return _channel->play();
}

tyrsound_Error SoundObject::stop()
{
    return _channel->stop();
}

tyrsound_Error SoundObject::pause()
{
    return _channel->pause();
}

tyrsound_Error SoundObject::seek(float seconds)
{
    return _decoder->seek(seconds);
}

float SoundObject::tell()
{
    return _decoder->tell();
}

tyrsound_Error SoundObject::setLoop(float seconds, int loops)
{
    return _decoder->setLoop(seconds, loops);
}

float SoundObject::getLength()
{
    return _decoder->getLength();
}

bool SoundObject::isPlaying()
{
    return _channel->isPlaying();
}

#include "tyrsound_end.h"
