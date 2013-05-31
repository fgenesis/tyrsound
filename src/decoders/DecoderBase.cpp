#include "DecoderBase.h"
#include "tyrsound_internal.h"

#include "tyrsound_begin.h"

void DecoderBase::destroy()
{
    this->~DecoderBase();
    Free(this);
}

#include "tyrsound_end.h"
