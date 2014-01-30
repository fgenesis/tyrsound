#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "tyrDeviceBase.h"
#include "tyrDecoderBase.h"
#include "tyrChannelBase.h"

#include "tyrsound_begin.h"

SoundObject *SoundObject::create(DecoderBase *decoder)
{
    void *p = Alloc(sizeof(SoundObject));
    return new(p) SoundObject(decoder);
}

void SoundObject::destroy()
{
    this->~SoundObject();
    Free(this);
}

SoundObject::~SoundObject()
{
    stop();
    _decoder->destroy();
}

SoundObject::SoundObject(DecoderBase *decoder)
: _idxInStore(unsigned(-1))
, _decoder(decoder)
, _channel(NULL)
, _dead(false)
, _update(false)
, _volume(1.0f)
, _speed(1.0f)
, _posX(0.0f)
, _posY(0.0f)
, _posZ(0.0f)
{
}

void SoundObject::_decode()
{
    while(_channel->wantData())
    {
        void *buf = NULL;
        size_t size = 0;
        _channel->getBuffer(&buf, &size);
        if(buf && size)
        {
            size_t filled = _decoder->fillBuffer(buf, size);
            tyrsound_Format fmt;
            _decoder->getFormat(&fmt);
            tyrsound_Error err = _channel->filledBuffer(filled, fmt);
            if(err != TYRSOUND_ERR_OK)
            {
                breakpoint();
                return;
            }
        }
        if(_decoder->isEOF())
            return;
    }
}

void SoundObject::update()
{
    if(!_update)
        return;

    _decode();
}

tyrsound_Error SoundObject::setVolume(float vol)
{
    _volume = vol;
    return _channel ? _channel->setVolume(vol) : TYRSOUND_ERR_NOT_APPLIED_TO_CHANNEL;
}

tyrsound_Error SoundObject::setSpeed(float speed)
{
    _speed = speed;
    return _channel ? _channel->setSpeed(speed) : TYRSOUND_ERR_NOT_APPLIED_TO_CHANNEL;
}

tyrsound_Error SoundObject::play()
{
    bool reserved = false;
    if(!_channel)
    {
        _channel = getDevice()->reserveChannel();
        reserved = true;
    }
    if(!_channel)
        return TYRSOUND_ERR_CHANNELS_FULL;

    tyrsound_Error err = TYRSOUND_ERR_OK;
    if(reserved)
    {
        err = _channel->prepare();
        if(err < TYRSOUND_ERR_OK)
        {
            if(reserved)
                getDevice()->retainChannel(_channel);
            return err;
        }

        // Initial decoding -- fill the buffer now and start playing immediately
        _decode();

        // If this was set when no channel pointer existed, do it now
        _channel->setVolume(_volume);
        _channel->setSpeed(_speed);
        _channel->setPosition(_posX, _posY, _posZ);
    }

    err = _channel->play();
    if(err < TYRSOUND_ERR_OK)
        stop();
    else
    {
        if(reserved)
            getDevice()->acquireChannel(_channel);

        registerUpdate(this);
    }

    return err;
}

tyrsound_Error SoundObject::stop()
{
    if(!_channel)
        return TYRSOUND_ERR_OK;

    unregisterUpdate(this);

    tyrsound_Error err = _channel->stop();
    getDevice()->retainChannel(_channel);
    _channel = NULL;

    return err;
}

tyrsound_Error SoundObject::pause()
{
    return _channel ? _channel->pause() : TYRSOUND_ERR_NOT_APPLIED_TO_CHANNEL;
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
    return _channel && _channel->isPlaying();
}

float SoundObject::getPlayPosition()
{
    return _channel ? _channel->getPlayPosition() : 0.0f;
}

tyrsound_Error SoundObject::setPosition(float x,  float y, float z)
{
    _posX = x;
    _posY = y;
    _posZ = z;
    return _channel ? _channel->setPosition(x, y, z) : TYRSOUND_ERR_NOT_APPLIED_TO_CHANNEL;
}

#include "tyrsound_end.h"
