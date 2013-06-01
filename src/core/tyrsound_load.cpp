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


static DecoderBase *createDecoder(tyrsound_Stream strm)
{
    tyrsound_Format fmt;
    tyrsound_getFormat(&fmt);
    for(unsigned int i = 0; TYRSOUND_DECODER_HOLDER::Size(); ++i)
        if(DecoderBase *decoder = TYRSOUND_DECODER_HOLDER::Get(i)->create(fmt, strm))
            return decoder;
    return NULL;
}


static tyrsound_Handle createSoundObject(tyrsound_Stream strm)
{
    if(!strm.read)
        return 0;

    ChannelBase *chan = getDevice()->getFreeChannel();
    if(!chan)
        return 0;

    DecoderBase *decoder = createDecoder(strm);
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



tyrsound_Handle tyrsound_load(tyrsound_Stream stream)
{
    return tyrsound::createSoundObject(stream);
}

