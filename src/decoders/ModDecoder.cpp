#include "tyrsound_internal.h"
#include "ModDecoder.h"
#include <stdio.h> // for SEEK_SET
#include "libopenmpt.h"

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DECODER(ModDecoder);


static size_t read_wrap(void *src, void *dst, size_t bytes)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)src;
    return (size_t)strm->read(dst, 1, bytes, strm->user);
}

static int seek_wrap(void *src, int64_t offset, int whence)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)src;
    return strm->seek ? strm->seek(strm->user, offset, whence) : -1;
}

static int close_wrap(void *datasource)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)datasource;
    return strm->close ? strm->close(strm->user) : 0;
}

static int64_t tell_wrap(void *datasource)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)datasource;
    return strm->tell ? (long)strm->tell(strm->user) : -1;
}

static void logfunc_wrap(const char * message, void * user)
{
    tyrsound_ex_message(TYRSOUND_MSG_INFO, message);
}

static const openmpt_stream_callbacks stream_callbacks =
{
    read_wrap,
    seek_wrap,
    tell_wrap
};


bool ModDecoder::checkMagic(const unsigned char *magic, size_t size)
{
    tyrsound_Stream strm;
    if(tyrsound_createMemStream(&strm, (void*)magic, size, NULL, 0) != TYRSOUND_ERR_OK)
        return false;
    return openmpt_could_open_propability(stream_callbacks, &strm, 0.25, logfunc_wrap, NULL) > 0;
}


ModDecoder::ModDecoder(const tyrsound_Stream& strm, const tyrsound_Format& fmt)
: _strm(strm)
, _fmt(fmt)
, _loopPoint(-1)
, _loopCount(0)
, _eof(false)
{
    _fmt.bigendian = false;
    _fmt.signedSamples = true;
    _fmt.sampleBits = 16;
    _fmt.channels = 2;
}

ModDecoder::~ModDecoder()
{
    if(_mod)
        openmpt_module_destroy((openmpt_module*)_mod);
    if(_strm.close)
        _strm.close(_strm.user);
}

ModDecoder *ModDecoder::create(const tyrsound_Format& fmt, const tyrsound_Stream& strm)
{

    void *mem = Alloc(sizeof(ModDecoder));
    if(!mem)
        goto fail;

    ModDecoder *decode = new(mem) ModDecoder(strm, fmt);
    if(!decode)
        goto fail;

    decode->_mod = openmpt_module_create(stream_callbacks, &decode->_strm, logfunc_wrap, NULL, NULL);
    if(!decode->_mod)
        goto fail;

    return decode;

fail:
    if(decode)
        decode->~ModDecoder();
    Free(mem);
    return NULL;
}

size_t ModDecoder::fillBuffer(void *buf, size_t size)
{
    size /= 4; // openmpt expects #samples (which are set to 16 bits)
    tyrsound_uint64 total = 0;
    while(!_eof && total < size)
    {
        size_t rd = openmpt_module_read_interleaved_stereo((openmpt_module*)_mod, _fmt.hz, size, (int16_t*)buf);
        if(!rd)
        {
            if(!_loopCount || _loopPoint < 0)
            {
                _eof = true;
                break;
            }

            if(_loopPoint >= 0)
            {
                // TODO: callback
                if(seek(_loopPoint) < TYRSOUND_ERR_OK)
                {
                    _eof = false;
                    break;
                }
                if(_loopCount > 0)
                    --_loopCount;
            }
        }
        total += rd;
    }
    return (size_t)total * 4;
}

float ModDecoder::getLength()
{
    return (float)openmpt_module_get_duration_seconds((openmpt_module*)_mod);
}

tyrsound_Error ModDecoder::seek(float seconds)
{
    openmpt_module_set_position_seconds((openmpt_module*)_mod, seconds);
    return TYRSOUND_ERR_OK; // FIXME
}

float ModDecoder::tell()
{
    return (float)openmpt_module_get_position_seconds((openmpt_module*)_mod);
}

tyrsound_Error ModDecoder::setLoop(float seconds, int loops)
{
    _loopPoint = seconds;
    _loopCount = loops;
    return TYRSOUND_ERR_OK;
}

float ModDecoder::getLoopPoint()
{
    return _loopPoint;
}

bool ModDecoder::isEOF()
{
    return _eof;
}

void ModDecoder::getFormat(tyrsound_Format *fmt)
{
    *fmt = _fmt;
}

#include "tyrsound_end.h"
