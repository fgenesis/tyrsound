#include "tyrsound.h"
#include "tyrsound_internal.h"
#include "SoundObject.h"
#include "DecoderBase.h"
#include "DeviceBase.h"

#include "tyrsound_begin.h"


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
        return NULL;

    ChannelBase *chan = getDevice()->getFreeChannel();
    if(!chan)
        return NULL;

    DecoderBase *decoder = createDecoder(strm);
    if(!decoder)
        return NULL;

    SoundObject *sound = SoundObject::create(decoder, chan);
    if(!sound)
    {
        decoder->destroy();
        return NULL;
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

