#include <al.h>
#include <alc.h>

#include "tyrsound_internal.h"
#include "OpenALChannel.h"
#include "OpenALDevice.h"

#include "tyrsound_begin.h"

#if TYRSOUND_IS_DEBUG
#  define CHECK_AL() do { ALint err_ = alGetError(); if(err_ != AL_NO_ERROR) {tyrsound_ex_messagef(TYRSOUND_MSG_ERROR, "AL error %d (0x%x)", err_, err_); breakpoint();} } while(0)
#else
#  define CHECK_AL()
#endif


OpenALChannel::OpenALChannel(OpenALDevice *dev)
: _aldev(dev)
, _sid(0)
, _bid(NULL)
, _pcmbuf(NULL)
, _pcmbufsize(0)
, _samplesDone(0)
, _inUse(false)
, _paused(false)
, _playing(false)
, _initial(true)
, _initialBufferIdx(0)
, _numBuffers(0)
, x_channelIndex(unsigned(-1))
{
}

OpenALChannel::~OpenALChannel()
{
    stop();
    if(_bid)
        alDeleteBuffers(_aldev->_cfg.numBuffers, _bid);
    alDeleteSources(1, &_sid);
    Free(_bid);
    Free(_pcmbuf);
}

tyrsound_Error OpenALChannel::prepare()
{
    if(_inUse)
        breakpoint();

    tyrsound_Error err = genBuffers();
    if(err != TYRSOUND_ERR_OK)
        return err;

    _inUse = true;
    _samplesDone = 0;
    _initial = true;
    _initialBufferIdx = 0;

    setInitialALValues();

    alSourceStop(_sid);
    alSourceRewind(_sid);
    alSourcei(_sid, AL_BUFFER, 0);

    return TYRSOUND_ERR_OK;
}

void OpenALChannel::setInitialALValues()
{
    setSpeed(1.0f);
    setVolume(1.0f);
    setPosition(0.0f, 0.0f, 0.0f);
}

tyrsound_Error OpenALChannel::genBuffers()
{
    ALenum err = AL_NO_ERROR;
    unsigned int numBuf = _aldev->_cfg.numBuffers;
    if(!_bid)
    {
        _numBuffers = 0;
        if(!numBuf)
            numBuf = 8;
        _bid = (ALuint*)Alloc(sizeof(ALuint) * numBuf);
        if(!_bid)
            return TYRSOUND_ERR_OUT_OF_MEMORY;
        alGenBuffers(numBuf, _bid);
        if(alGetError() != AL_NO_ERROR)
            return TYRSOUND_ERR_UNSPECIFIED;
        _numBuffers = numBuf;
    }
    if(!_pcmbuf)
    {
        _pcmbufsize = 0;
        size_t pcmSize = _aldev->_cfg.bufferSize;
        if(!pcmSize)
            pcmSize = 16 * 1024;
        _pcmbuf = (char*)Alloc(pcmSize);
        if(!_pcmbuf)
            return TYRSOUND_ERR_OUT_OF_MEMORY;
        _pcmbufsize = pcmSize;
    }

    err = alGetError();
    return (err == AL_NO_ERROR && _bid[0] != 0) ? TYRSOUND_ERR_OK : TYRSOUND_ERR_UNSPECIFIED;
}

bool OpenALChannel::wantData()
{
    if(!_inUse)
        return false;
    if(_initial)
        return true;
    ALint done = 0;
    alGetSourcei(_sid, AL_BUFFERS_PROCESSED, &done);
    return done != 0;
}

void OpenALChannel::getBuffer(void **buf, size_t *size)
{
   *buf = _pcmbuf;
   *size = _pcmbufsize;
}

void OpenALChannel::update()
{
    if(!_inUse || _initial)
        return;

    ALint state = 0;
    alGetSourceiv(_sid, AL_SOURCE_STATE, &state);
    CHECK_AL();
    switch(state)
    {
        case AL_STOPPED:
            stop();
            break;
        case AL_INITIAL:
        case AL_PAUSED:
            if(_playing && !_paused)
                play();
            break;
        case AL_PLAYING:
            if(!_playing && _paused)
                pause();
            break;
        default:
            breakpoint();
            break;
    }
}

tyrsound_Error OpenALChannel::filledBuffer(size_t size, const tyrsound_Format& fmt)
{
    if(size > _pcmbufsize)
        return TYRSOUND_ERR_SHIT_HAPPENED;

    if (size > 0)
    {
        unsigned int bytesPerSample = 0;
        ALenum alformat = OpenALDevice::getALFormat(fmt, &bytesPerSample);
        if(alformat < 0)
        {
            breakpoint();
            return TYRSOUND_ERR_UNSUPPORTED_FORMAT;
        }

        _hz = fmt.hz;
        ALuint bid = 0;

        if(_initial)
        {
            // Initially, fill & queue all buffers
            if(_initialBufferIdx < _numBuffers)
                bid = _bid[_initialBufferIdx];
            else
                return TYRSOUND_ERR_SHIT_HAPPENED;
        }
        else
        {
            // Later, pop off used buffers and re-queue
            alSourceUnqueueBuffers(_sid, 1, &bid);
            if(!bid)
                return TYRSOUND_ERR_NOT_READY;
        }

        if(size % bytesPerSample)
            breakpoint();

        CHECK_AL();
        alBufferData(bid, alformat, _pcmbuf, size, _hz);
        CHECK_AL();
        alSourceQueueBuffers(_sid, 1, &bid);
        CHECK_AL();

        if(_initial)
        {
            ++_initialBufferIdx;
            if(_initialBufferIdx >= _numBuffers)
                _initial = false;
        }
        else
        {
            _samplesDone += size / bytesPerSample;
        }
    }
    else
    {
        // end of stream?
        if(_initial && _initialBufferIdx) // if at least one packet was buffered, we're ready to play
            _initial = false;
    }

    return TYRSOUND_ERR_OK;
}

tyrsound_Error OpenALChannel::setVolume(float vol)
{
    _volume = vol;
    alSourcef(_sid, AL_GAIN, vol);
    return alGetError() == AL_NO_ERROR ? TYRSOUND_ERR_OK : TYRSOUND_ERR_UNSPECIFIED;
}

tyrsound_Error OpenALChannel::setSpeed(float speed)
{
    _speed = speed;
    alSourcef(_sid, AL_PITCH, speed);
    return alGetError() == AL_NO_ERROR ? TYRSOUND_ERR_OK : TYRSOUND_ERR_UNSPECIFIED;
}

tyrsound_Error OpenALChannel::pause()
{
    if(!_playing)
        return TYRSOUND_ERR_NOT_READY;

    alSourcePause(_sid);
    CHECK_AL();
    _paused = true;
    return TYRSOUND_ERR_OK;
}

tyrsound_Error OpenALChannel::play()
{
    alSourcePlay(_sid);
    if(alGetError() != AL_NO_ERROR)
        return TYRSOUND_ERR_UNSPECIFIED;

    _paused = false;
    _playing = true;

    return TYRSOUND_ERR_OK;
}

tyrsound_Error OpenALChannel::stop()
{
    if(!_inUse)
        return TYRSOUND_ERR_OK;

    _paused = false;
    _playing = false;
    alSourceStop(_sid);
    CHECK_AL();

    ALint queued = 0;
    alGetSourcei(_sid, AL_BUFFERS_QUEUED, &queued);
    CHECK_AL();
    for (int i = 0; i < queued; i++)
    {
        ALuint bid;
        alSourceUnqueueBuffers(_sid, 1, &bid);
        CHECK_AL();
        alDeleteBuffers(1, &bid);
        CHECK_AL();
    }

    alSourcei(_sid, AL_BUFFER, 0);
    CHECK_AL();
    Free(_pcmbuf);
    _pcmbuf = NULL;
    _pcmbufsize = 0;
    Free(_bid);
    _bid = NULL;
    _numBuffers = 0;
    _inUse = false;
    _samplesDone = 0;
    return TYRSOUND_ERR_OK;
}

bool OpenALChannel::isPlaying()
{
    return _playing && !_paused;
}

tyrsound_Error OpenALChannel::setPosition(float x, float y, float z)
{
    alSource3f(_sid, AL_POSITION, x, y, z);
    return alGetError() == AL_NO_ERROR ? TYRSOUND_ERR_OK : TYRSOUND_ERR_UNSPECIFIED;
}

// FIXME: return correct value on loop / seek
float OpenALChannel::getPlayPosition()
{
    ALint offs = 0;
    alGetSourcei(_sid, AL_SAMPLE_OFFSET, &offs);
    CHECK_AL();
    tyrsound_int64 totalDone = _samplesDone + offs;
    return float((double)totalDone / (double)_hz);
}


#include "tyrsound_end.h"
