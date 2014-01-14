#include <stdio.h>
#include "tyrsound.h"
#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "tyrDecoderBase.h"
#include "tyrDeviceBase.h"
#include "../decoders/RawDecoder.h"

#include "tyrsound_begin.h"

#define TYRSOUND_DECODER_HOLDER RegistrationHolder<DecoderFactoryBase*, 128>


void tyrsound_ex_registerDecoder(DecoderFactoryBase *f)
{
    TYRSOUND_DECODER_HOLDER::Register(f);
}

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


static DecoderBase *createDecoder(const tyrsound_Stream& strm, const tyrsound_Format& fmt, bool skipMagic)
{
    const unsigned int numDecoders = TYRSOUND_DECODER_HOLDER::Size();
    if(!numDecoders)
        return NULL;

    tyrsound_int64 pos = strm.tell ? strm.tell(strm.user) : 0;
    unsigned char magic[513];
    size_t magicsize = 0;
    if(!skipMagic)
    {
        if((magicsize = (size_t)strm.read(&magic[0], 1, 512, strm.user)) < 8)
            return NULL; // too short
        magic[magicsize] = 0; // for strcmp() and family
    }

    strm.seek(strm.user, pos, SEEK_SET);
    for(unsigned int i = 0; i < numDecoders; ++i)
    {
        if(skipMagic || TYRSOUND_DECODER_HOLDER::Get(i)->checkMagic(magic, magicsize))
        {
            // The first few bytes look okay, try to create the decoder
            if(DecoderBase *decoder = TYRSOUND_DECODER_HOLDER::Get(i)->create(fmt, strm))
                return decoder;

            // Didn't work -- try next decoder, but make sure the stream is at the previous position
            strm.seek(strm.user, pos, SEEK_SET);
        }
    }
    return NULL;
}

static tyrsound_Handle createSoundObjectWithDecoder(DecoderBase *decoder)
{
    SoundObject *sound = SoundObject::create(decoder);
    if(!sound)
    {
        decoder->destroy();
        return 0;
    }

    tyrsound_Handle handle = registerSoundObject(sound);
    if(handle == TYRSOUND_NULLHANDLE)
    {
        decoder->destroy();
        sound->destroy();
    }

    return handle;
}

static tyrsound_Handle createSoundObject(const tyrsound_Stream& strm, const tyrsound_Format& fmt, bool skipMagic)
{
    DecoderBase *decoder = createDecoder(strm, fmt, skipMagic);
    if(!decoder)
        return 0;

    return createSoundObjectWithDecoder(decoder);
}

tyrsound_Handle loadStream(tyrsound_Stream stream, const tyrsound_Format *fmt, bool skipMagic)
{
    if(!stream.read)
        return TYRSOUND_ERR_INVALID_VALUE;

    if(!stream.seek)
    {
        tyrsound_Stream srcbuf;
        tyrsound_Error err = tyrsound_bufferStream(&srcbuf, NULL, stream);
        if(err != TYRSOUND_ERR_OK)
            return err;
        stream = srcbuf;
    }

    tyrsound_Format f;
    if(!fmt)
        tyrsound_getFormat(&f);
    return tyrsound::createSoundObject(stream, fmt ? *fmt : f, false);
}



#include "tyrsound_end.h"



tyrsound_Handle tyrsound_load(tyrsound_Stream stream)
{
    return tyrsound::loadStream(stream, NULL, false);
}

tyrsound_Handle tyrsound_loadTryHarder(tyrsound_Stream stream, const tyrsound_Format *fmt, int tryHard)
{
    return tyrsound::loadStream(stream, fmt, !!tryHard);
}


tyrsound_Handle tyrsound_fromDecoder(void *decoder)
{
    return tyrsound::createSoundObjectWithDecoder((tyrsound::DecoderBase*)decoder);
}

tyrsound_Error tyrsound_decodeStream(tyrsound_Stream dst, tyrsound_Format *dstfmt, tyrsound_Stream src, tyrsound_Format *srcfmt, int tryHard, float maxSeconds)
{
    if(!src.read || !dst.write || !dstfmt)
        return TYRSOUND_ERR_INVALID_VALUE;

    if(!src.seek)
    {
        tyrsound_Stream srcbuf;
        tyrsound_Error err = tyrsound_bufferStream(&srcbuf, NULL, src);
        if(err != TYRSOUND_ERR_OK)
            return err;
        src = srcbuf;
    }

    tyrsound_Format f;
    if(!srcfmt)
        tyrsound_getFormat(&f);

    tyrsound::DecoderBase *decoder = tyrsound::createDecoder(src, srcfmt ? *srcfmt : f, !!tryHard);
    if(!decoder)
        TYRSOUND_ERR_UNSUPPORTED;

    if(decoder->getLength() < 0 && !maxSeconds)
    {
        decoder->destroy();
        return TYRSOUND_ERR_INFINITE;
    }

    char buf[4096];
    while(!decoder->isEOF())
    {
        if(maxSeconds > 0 && decoder->tell() >= maxSeconds)
            break;
        size_t filled = decoder->fillBuffer(buf, sizeof(buf));
        tyrsound_int64 written = dst.write(buf, 1, filled, dst.user);
        if(!written)
            return TYRSOUND_ERR_NOT_READY;
    }

    decoder->getFormat(dstfmt);

    decoder->destroy();

    return TYRSOUND_ERR_OK;
}



static void _deleteMem(void *p)
{
    tyrsound::Free(p);
}

tyrsound_Handle tyrsound_loadRawBuffer(void *buf, size_t bytes, const tyrsound_Format *fmt)
{
    void *p = tyrsound::Alloc(bytes);
    if(!p)
        return 0;

    memcpy(p, buf, bytes);

    tyrsound_Stream strm;
    tyrsound_createMemStream(&strm, p, bytes, _deleteMem, 0);

    return tyrsound_loadRawStream(strm, fmt);
}

tyrsound_Handle tyrsound_loadRawStream(tyrsound_Stream strm, const tyrsound_Format *fmt)
{
    tyrsound_Format f;
    if(!fmt)
        tyrsound_getFormat(&f);

    tyrsound::RawDecoder *decoder = tyrsound::RawDecoder::create(fmt ? *fmt : f, strm);
    return tyrsound::createSoundObjectWithDecoder(decoder);
}

