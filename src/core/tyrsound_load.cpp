#include <stdio.h> // for SEEK_SET
#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "tyrDecoderBase.h"
#include "tyrDeviceBase.h"
#include "../decoders/RawDecoder.h"

#include "tyrsound_begin.h"

#define TYRSOUND_DECODER_HOLDER tyrsound::RegistrationHolder<tyrsound::DecoderFactoryBase*, 128>

void initDecoders()
{
    const unsigned int numDecoders = TYRSOUND_DECODER_HOLDER::Size();
    for(unsigned int i = 0; i < numDecoders; ++i)
        TYRSOUND_DECODER_HOLDER::Get(i)->staticInit();
}

void shutdownDecoders()
{
    const unsigned int numDecoders = TYRSOUND_DECODER_HOLDER::Size();
    for(unsigned int i = 0; i < numDecoders; ++i)
        TYRSOUND_DECODER_HOLDER::Get(i)->staticShutdown();
}

static DecoderBase *createDecoder(tyrsound_Stream& strm, const tyrsound_Format& fmt, bool skipMagic)
{
    const unsigned int numDecoders = TYRSOUND_DECODER_HOLDER::Size();
    if(!numDecoders)
        return NULL;

    tyrsound_int64 pos = strm.tell ? strm.tell(strm.user) : 0;
    const unsigned MAGICSIZE = 3*512; // TODO: ask decoders for req. size and take maximum
    unsigned char magic[MAGICSIZE+1];
    size_t magicsize = 0;
    if(!skipMagic)
    {
        if((magicsize = (size_t)strm.read(&magic[0], 1, MAGICSIZE, strm.user)) < 8)
            return NULL; // too short
        magic[magicsize] = 0; // for strcmp() and family
    }

    strm.seek(strm.user, pos, SEEK_SET);
    for(unsigned int i = 0; i < numDecoders; ++i)
    {
        if(skipMagic || TYRSOUND_DECODER_HOLDER::Get(i)->checkMagic(magic, magicsize))
        {
            tyrsound_Format copy = fmt;
            copy.isfloat = -fmt.isfloat; // decoder is expected to set this to positive if it supports float output
            // The first few bytes look okay, try to create the decoder
            if(DecoderBase *decoder = TYRSOUND_DECODER_HOLDER::Get(i)->create(copy, strm))
                return decoder;

            // Didn't work -- try next decoder, but make sure the stream is at the previous position
            strm.seek(strm.user, pos, SEEK_SET);
        }
    }
    tyrsound_ex_message(TYRSOUND_MSG_INFO, "Stream format not recognized");
    return NULL;
}

static tyrsound_Sound createSoundObjectWithDecoder(DecoderBase *decoder)
{
    SoundObject *sound = SoundObject::create(decoder);
    if(!sound)
    {
        decoder->destroy();
        return TYRSOUND_NULL_SOUND;
    }

    tyrsound_Sound handle = registerSoundObject(sound);
    if(handle == TYRSOUND_NULL_SOUND)
    {
        decoder->destroy();
        sound->destroy();
    }

    return handle;
}

static tyrsound_Sound createSoundObject(tyrsound_Stream& strm, const tyrsound_Format& fmt, bool skipMagic)
{
    DecoderBase *decoder = createDecoder(strm, fmt, skipMagic);
    if(!decoder)
        return TYRSOUND_NULL_SOUND;

    return createSoundObjectWithDecoder(decoder);
}

tyrsound_Sound loadStream(tyrsound_Stream *stream, const tyrsound_Format *fmt, bool skipMagic)
{
    if(!stream)
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "stream == NULL");
        return TYRSOUND_NULL_SOUND;
    }
    
    if(!stream->read)
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Stream needs a read function");
        return TYRSOUND_NULL_SOUND;
    }
    
    tyrsound_Stream useStream = *stream;

    // FIXME: This should go at some point.
    if(!stream->seek)
    {
        tyrsound_ex_message(TYRSOUND_MSG_WARNING, "Stream is not seekable, prebuffering");
        tyrsound_Error err = tyrsound_bufferStream(&useStream, NULL, stream);
        if(err != TYRSOUND_ERR_OK)
            return TYRSOUND_NULL_SOUND;
    }

    tyrsound_Format f;
    if(!fmt)
        tyrsound_getFormat(&f);
    return tyrsound::createSoundObject(useStream, fmt ? *fmt : f, false);
}



#include "tyrsound_end.h"


void tyrsound_ex_registerDecoder(tyrsound::DecoderFactoryBase *f)
{
    TYRSOUND_DECODER_HOLDER::Register(f);
}

TYRSOUND_DLL_EXPORT tyrsound_Sound tyrsound_load(tyrsound_Stream *stream)
{
    return tyrsound::loadStream(stream, NULL, false);
}

TYRSOUND_DLL_EXPORT tyrsound_Sound tyrsound_loadEx(tyrsound_Stream *stream, const tyrsound_Format *fmt, int tryHard)
{
    return tyrsound::loadStream(stream, fmt, !!tryHard);
}


TYRSOUND_DLL_EXPORT tyrsound_Sound tyrsound_fromDecoder(void *decoder)
{
    return tyrsound::createSoundObjectWithDecoder((tyrsound::DecoderBase*)decoder);
}

TYRSOUND_DLL_EXPORT tyrsound_Error tyrsound_decodeStream(tyrsound_Stream *dst, tyrsound_Format *dstfmt, tyrsound_Stream *src, tyrsound_Format *srcfmt, int tryHard, float maxSeconds)
{
    if(!src || !dst)
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "stream == NULL");
        return TYRSOUND_ERR_INVALID_VALUE;
    }
    if(!src->read)
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "src stream needs read function");
        return TYRSOUND_ERR_INVALID_VALUE;
    }
    if(!dst->write)
    {
        tyrsound_ex_message(TYRSOUND_MSG_ERROR,  "dst stream needs write function");
        return TYRSOUND_ERR_INVALID_VALUE;
    }


    tyrsound_Stream useStream = *src;
    if(!src->seek)
    {
        tyrsound_Error err = tyrsound_bufferStream(&useStream, NULL, src);
        if(err != TYRSOUND_ERR_OK)
            return err;
    }

    tyrsound_Format f;
    if(!srcfmt)
        tyrsound_getFormat(&f);

    tyrsound::DecoderBase *decoder = tyrsound::createDecoder(useStream, srcfmt ? *srcfmt : f, !!tryHard);
    if(!decoder)
        return TYRSOUND_ERR_UNSUPPORTED;

    if(decoder->getLength() < 0 && !maxSeconds)
    {
        decoder->destroy();
        tyrsound_ex_message(TYRSOUND_MSG_ERROR, "Stream is infinite and would decode forever");
        return TYRSOUND_ERR_INFINITE;
    }

    char buf[4096];
    while(!decoder->isEOF())
    {
        if(maxSeconds > 0 && decoder->tell() >= maxSeconds)
            break;
        size_t filled = decoder->fillBuffer(buf, sizeof(buf));
        tyrsound_int64 written = dst->write(buf, 1, filled, dst->user);
        if(!written)
            return TYRSOUND_ERR_NOT_READY;
    }

    if(dstfmt)
        decoder->getFormat(dstfmt);

    decoder->destroy();

    return TYRSOUND_ERR_OK;
}



static void _deleteMem(void *p)
{
    tyrsound::Free(p);
}

TYRSOUND_DLL_EXPORT tyrsound_Sound tyrsound_loadRawBuffer(void *buf, size_t bytes, const tyrsound_Format *fmt)
{
    void *p = tyrsound::Alloc(bytes);
    if(!p)
        return TYRSOUND_NULL_SOUND;

    memcpy(p, buf, bytes);

    tyrsound_Stream strm;
    tyrsound_createMemStream(&strm, p, bytes, _deleteMem, 0);

    return tyrsound_loadRawStream(&strm, fmt);
}

TYRSOUND_DLL_EXPORT tyrsound_Sound tyrsound_loadRawBufferNoCopy(void *buf, size_t bytes, const tyrsound_Format *fmt)
{
    tyrsound_Stream strm;
    tyrsound_createMemStream(&strm, buf, bytes, NULL, 0);
    return tyrsound_loadRawStream(&strm, fmt);
}

TYRSOUND_DLL_EXPORT tyrsound_Sound tyrsound_loadRawStream(tyrsound_Stream *strm, const tyrsound_Format *fmt)
{
    tyrsound_Format f;
    if(!fmt)
        tyrsound_getFormat(&f);

    tyrsound::RawDecoder *decoder = tyrsound::RawDecoder::create(fmt ? *fmt : f, *strm);
    return tyrsound::createSoundObjectWithDecoder(decoder);
}

