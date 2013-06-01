#include "tyrChannelBase.h"

#include "tyrsound_begin.h"

void ChannelBase::destroy()
{
    this->~ChannelBase();
    Free(this);
}

#include "tyrsound_end.h"
