#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "ChannelGroup.h"
#include "tyrDeviceBase.h"
#include "tyrDecoderBase.h"
#include "tyrChannelBase.h"

#include "tyrsound_begin.h"


static void _fixDecoderFormat(tyrsound_Format& fmt)
{
    if(fmt.isfloat < 0) // if the decoder ignored -1, assume it does not support float
        fmt.isfloat = 0;
    else if(fmt.isfloat)
    {
        fmt.sampleBits = sizeof(float) * 8;
        fmt.signedSamples = -1; // irrelevant for float
    }
}


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
    detachFromGroup();
    stop();
    _decoder->destroy();
}

SoundObject::SoundObject(DecoderBase *decoder) : ReferencedPlayable(TY_SOUND)
, group(NULL)
, _decoder(decoder)
, _channel(NULL)
, _update(false)
, _autofree(false)
, _initial(true)
, _volume(1.0f)
, _speed(1.0f)
, _posX(0.0f)
, _posY(0.0f)
, _posZ(0.0f)
, groupvolume(1.0f)
, groupspeed(1.0f)
, groupx(0.0f), groupy(1.0f), groupz(1.0f)
{
}

void SoundObject::detachFromGroup()
{
    if(group)
        group->detachSound(this);
}

void SoundObject::_decode()
{
    while(_channel->wantData())
    {
        tyrsound_Format fmt;

        if(_decoder->isEOF())
        {
            // notify channel that we're at EOF
            // leave fmt uninitialized, is not supposed to be used with zero size
            _channel->filledBuffer(0, fmt);
            break;
        }

        void *buf = NULL;
        size_t size = 0;
        _channel->getBuffer(&buf, &size);
        if(buf && size)
        {
            size_t filled = _decoder->fillBuffer(buf, size);
            tyrsound_ex_messagef(TYRSOUND_MSG_SPAM, "Decoded %u bytes", (unsigned int)filled);
            _decoder->getFormat(&fmt);
            _fixDecoderFormat(fmt);
            tyrsound_Error err = _channel->filledBuffer(filled, fmt);
            if(err != TYRSOUND_ERR_OK)
            {
                breakpoint();
                return;
            }
        }
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
    return updateVolume();
}

tyrsound_Error SoundObject::setGroupVolume(float vol)
{
    groupvolume = vol;
    return updateVolume();
}

tyrsound_Error SoundObject::updateVolume()
{
    return _channel ? _channel->setVolume(_volume * groupvolume) : TYRSOUND_ERR_NOT_APPLIED_TO_CHANNEL;
}

tyrsound_Error SoundObject::setSpeed(float speed)
{
    _speed = speed;
    return updateSpeed();
}

tyrsound_Error SoundObject::setGroupSpeed(float speed)
{
    groupspeed = speed;
    return updateSpeed();
}

tyrsound_Error SoundObject::updateSpeed()
{
    return _channel ? _channel->setSpeed(_speed * groupspeed) : TYRSOUND_ERR_NOT_APPLIED_TO_CHANNEL;
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
    {
        _initial = false;
        return TYRSOUND_ERR_CHANNELS_FULL;
    }

    tyrsound_Error err = TYRSOUND_ERR_OK;
    if(reserved)
    {
        err = _channel->prepare();
        if(err < TYRSOUND_ERR_OK)
        {
            if(reserved)
                getDevice()->retainChannel(_channel);
            _initial = false;
            return err;
        }

        // Initial decoding -- fill the buffer now and start playing immediately
        _decode();

        // If this was set when no channel pointer existed, do it now
        updateVolume();
        updateSpeed();
        updatePosition();
    }

    err = _channel->play();
    if(err < TYRSOUND_ERR_OK)
        stop();
    else
    {
        if(reserved)
            getDevice()->acquireChannel(_channel);

        _update = true;
    }

    _initial = false;
    return err;
}

tyrsound_Error SoundObject::stop()
{
    if(!_channel)
        return TYRSOUND_ERR_OK;

    _update = false;

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

int SoundObject::isPlaying()
{
    return _channel && _channel->isPlaying();
}

int SoundObject::isStopped()
{
    return !_initial  && (!_channel || _channel->isStopped());
}


float SoundObject::getPlayPosition()
{
    return _channel ? _channel->getPlayPosition() : 0.0f;
}

tyrsound_Error SoundObject::setPosition(float x, float y, float z)
{
    _posX = x;
    _posY = y;
    _posZ = z;
    return updatePosition();
}

tyrsound_Error SoundObject::setGroupPosition(float x, float y, float z)
{
    groupx = x;
    groupy = y;
    groupz = z;
    return updatePosition();
}

tyrsound_Error SoundObject::updatePosition()
{
    return _channel
        ? _channel->setPosition(_posX + groupx, _posY + groupx, _posZ + groupz)
        : TYRSOUND_ERR_NOT_APPLIED_TO_CHANNEL;
}

#include "tyrsound_end.h"
