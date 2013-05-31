#include "OpenALChannel.h"
#include "tyrsound_internal.h"

#include "tyrsound_begin.h"


OpenALChannel::OpenALChannel()
//: _buffer(NULL)
{
}

OpenALChannel::~OpenALChannel()
{
    //Free(_buffer);
}

OpenALChannel *OpenALChannel::create(const tyrsound_Format& fmt)
{
    /*size_t bufsize = fmt ? fmt->bufferSize : 0;
    bufsize = bufsize ? bufsize : (8 * 1024);
    void *mem = Alloc(sizeof(NullOutput));
    if(!mem)
        return NULL;
    NullOutput *output = new(mem) NullOutput();

    mem = Alloc(bufsize);
    if(!mem)
    {
        output->destroy();
        return NULL;
    }

    output->_buffer = mem;

    return output;*/

    return NULL;
}

bool OpenALChannel::wantData()
{
    return false;
}

void OpenALChannel::getBuffer(void **buf, size_t *size)
{
    //*buf = _buffer;
    //*size = _bufsize;
}

void OpenALChannel::update()
{
}

void OpenALChannel::filledBuffer(size_t size)
{
}

tyrsound_Error OpenALChannel::setVolume(float vol)
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

tyrsound_Error OpenALChannel::setSpeed(float vol)
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

tyrsound_Error OpenALChannel::pause()
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

tyrsound_Error OpenALChannel::play()
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

tyrsound_Error OpenALChannel::stop()
{
    return TYRSOUND_ERR_UNSUPPORTED;
}

bool OpenALChannel::isPlaying() const
{
    return false;
}


#include "tyrsound_end.h"
