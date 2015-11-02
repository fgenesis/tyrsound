#include "tyrDecoderBase.h"
#include "tyrsound_internal.h"

#include "tyrsound_begin.h"

DecoderBase::DecoderBase(const tyrsound_Format& fmt)
: _fmt(fmt)
{
}

void DecoderBase::destroy()
{
    this->~DecoderBase();
    Free(this);
}

tyrsound_Error DecoderBase::seek(float seconds)
{
    tyrsound_uint64 sample = tyrsound_uint64(seconds * _fmt.hz);
    return seekSample(sample);
}

float DecoderBase::tell()
{
    tyrsound_uint64 pos = tellSample();
    return pos / float(_fmt.hz);
}


#include "tyrsound_end.h"
