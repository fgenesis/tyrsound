#include <stdio.h>
#include "tyrsound.h"
#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "tyrDecoderBase.h"
#include "tyrDeviceBase.h"

#include "tyrsound_begin.h"

#define TYRSOUND_DECODER_HOLDER RegistrationHolder<DecoderFactoryBase*, 128>


void tyrsound_ex_registerDecoder(DecoderFactoryBase *f)
{
    TYRSOUND_DECODER_HOLDER::Register(f);
}


static DecoderBase *createDecoder(tyrsound_Stream strm, const tyrsound_Format& fmt)
{
    tyrsound_int64 pos = strm.tell ? strm.tell(strm.user) : 0;
    const unsigned int numDecoders = TYRSOUND_DECODER_HOLDER::Size();
    for(unsigned int i = 0; i < numDecoders; ++i)
    {
        if(DecoderBase *decoder = TYRSOUND_DECODER_HOLDER::Get(i)->create(fmt, strm))
            return decoder;
        strm.seek(strm.user, pos, SEEK_SET);
        // FIXME: try to support non-seekable streams somehow
    }
    return NULL;
}


static tyrsound_Handle createSoundObject(tyrsound_Stream strm, const tyrsound_Format& fmt)
{
    ChannelBase *chan = getDevice()->getFreeChannel();
    if(!chan)
        return 0;

    DecoderBase *decoder = createDecoder(strm, fmt);
    if(!decoder)
        return 0;

    SoundObject *sound = SoundObject::create(decoder, chan);
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


#include "tyrsound_end.h"



tyrsound_Handle tyrsound_load(tyrsound_Stream stream, const tyrsound_Format *fmt)
{
    if(!stream.read)
        return TYRSOUND_ERR_INVALID_VALUE;
    if(!stream.seek)
        return TYRSOUND_ERR_INVALID_VALUE;

    tyrsound_Format f;
    if(!fmt)
        tyrsound_getFormat(&f);
    return tyrsound::createSoundObject(stream, fmt ? *fmt : f);
}

tyrsound_Error tyrsound_decodeStream(tyrsound_Stream dst, tyrsound_Format *dstfmt, tyrsound_Stream src, tyrsound_Format *srcfmt)
{
    if(!src.read || !dst.write)
        return TYRSOUND_ERR_INVALID_VALUE;
    if(!src.seek)
        return TYRSOUND_ERR_INVALID_VALUE;

    tyrsound_Format f;
    if(!srcfmt)
        tyrsound_getFormat(&f);

    tyrsound::DecoderBase *decoder = tyrsound::createDecoder(src, srcfmt ? *srcfmt : f);
    if(!decoder)
        TYRSOUND_ERR_UNSUPPORTED;

    char buf[2048];
    while(!decoder->isEOF())
    {
        decoder->fillBuffer(buf, sizeof(buf));
        tyrsound_int64 written = dst.write(buf, 1, sizeof(buf), dst.user);
        if(written != sizeof(buf))
            return TYRSOUND_ERR_NOT_READY;
    }

    if(dstfmt)
        decoder->getFormat(dstfmt);

    decoder->destroy();

    return TYRSOUND_ERR_OK;
}

