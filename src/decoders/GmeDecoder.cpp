#include "tyrsound_internal.h"
#include "GmeDecoder.h"
#include <stdio.h>
#include <gme.h>

#include "tyrsound_begin.h"

TYRSOUND_REGISTER_DECODER(GmeDecoder);


static gme_err_t reader_wrap( void* your_data, void* out, int count)
{
    tyrsound_Stream *strm = (tyrsound_Stream*)your_data;
    tyrsound_uint64 done = strm->read(out, 1, count, strm->user);
    return done == count ? NULL : "truncated file";
}

#define EMU ((gme_t*)_emu)

GmeDecoder::GmeDecoder(void *emu, const tyrsound_Format& fmt)
: _emu(emu)
, _loopPoint(-1.0f)
, _eof(false)
, _loopCount(0)
, _totaltime(-1.0f)
{
    /*
    // 16 bits if not specified
    unsigned int bits = fmt.sampleBits == 0 ? 16 : fmt.sampleBits;
    _sampleWordSize = (bits <= 8 ? 1 : 2);
    _totaltime = (float)ov_time_total(&((GmeDecoderState*)state)->vf, -1);
    _seekable = ov_seekable(&((GmeDecoderState*)state)->vf) != 0;
    _fmt = fmt;
    vorbis_info *vi = ov_info(&((GmeDecoderState*)state)->vf, -1);
    _fmt.channels = vi->channels;
    _fmt.hz = vi->rate;
    _fmt.sampleBits = bits;
    */

    _fmt = fmt;
    _fmt.bigendian = 0;
    _fmt.signedSamples = 1;
    _fmt.sampleBits = 16;
    _fmt.channels = 2;

    gme_info_t *info = NULL;
    gme_track_info(EMU, &info, 0);
    if(info)
    {
        int len = info->length;
        if(len <= 0)
        {
            len = 0;
            if(info->intro_length >= 0)
                len += info->intro_length;
            if(info->loop_length >= 0)
                len += info->loop_length;
        }
        _totaltime = len > 0 ? len / 1000.0f : -1;
    }
    gme_free_info(info);
}

GmeDecoder::~GmeDecoder()
{
    gme_delete(EMU);
}

bool GmeDecoder::checkMagic(const char *magic, size_t size)
{
    gme_type_t file_type = gme_identify_extension(gme_identify_header(magic));
    return file_type != NULL;
}

GmeDecoder *GmeDecoder::create(const tyrsound_Format& fmt, tyrsound_Stream strm)
{
    char header[4];
    if(strm.read(header, 1, 4, strm.user) != 4)
        return NULL;
    gme_type_t file_type = gme_identify_extension(gme_identify_header(header));
    if(!file_type)
        return NULL;

    gme_t *emu = gme_new_emu(file_type, fmt.hz);
    if(!emu)
        return NULL;

    strm.seek(strm.user, 0, SEEK_SET);

    // Because GME wants to know the actual file size, we need to preload the buffer if it's not known.
    tyrsound_int64 totalsize = strm.remain ? strm.remain(strm.user) : -1;
    if(totalsize < 0)
    {
        tyrsound_Stream sbuf;
        if(tyrsound_createGrowingBuffer(&sbuf, 128) != TYRSOUND_ERR_OK)
            return NULL;
        tyrsound_uint64 totalsizeu;
        if(tyrsound_bufferStream(&sbuf, &totalsizeu, strm) != TYRSOUND_ERR_OK)
            return NULL;
        totalsize = totalsizeu;
        strm.close(strm.user);
        strm = sbuf;
    }

    gme_err_t gmerr = gme_load_custom(emu, reader_wrap, (long)totalsize, &strm);
    if(gmerr)
        return NULL;

    void *mem = Alloc(sizeof(GmeDecoder));
    if(!mem)
    {
        gme_delete(emu);
        return NULL;
    }

    gme_start_track(emu, 0);

    GmeDecoder *decoder = new(mem) GmeDecoder(emu, fmt);
    return decoder;
}

size_t GmeDecoder::fillBuffer(void *buf, size_t size)
{
    size_t samples = size / 2;
    gme_err_t err = gme_play(EMU, samples, (short*)buf);
    if(err)
    {
        _eof = true;
        return 0;
    }

    if(_loopPoint >= 0.0f && _loopCount != 0)
    {
        if(isEOF())
        {
            if(_loopCount > 0)
                --_loopCount;
            seek(_loopPoint);
        }
    }

    return samples * 2;
}

tyrsound_Error GmeDecoder::seek(float seconds)
{
    gme_err_t err = gme_seek(EMU, int(seconds * 1000.0f));
    if(err)
        return TYRSOUND_ERR_UNSPECIFIED;
    
    _eof = false;
    return TYRSOUND_ERR_OK;
}

tyrsound_Error GmeDecoder::setLoop(float seconds, int loops)
{
    _loopPoint = seconds;
    _loopCount = loops;
    return TYRSOUND_ERR_OK;
}

float GmeDecoder::getLoopPoint()
{
    return _loopPoint;
}


float GmeDecoder::getLength()
{
    return _totaltime;
}

float GmeDecoder::tell()
{
    return gme_tell(EMU) / 1000.0f;
}

bool GmeDecoder::isEOF()
{
    return _eof || gme_track_ended(EMU);
}

void GmeDecoder::getFormat(tyrsound_Format *fmt)
{
    *fmt = _fmt;
}


#include "tyrsound_end.h"

