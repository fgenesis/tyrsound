#include "ChannelBase.h"
#include "tyrsound_internal.h"

#include "tyrsound_begin.h"

void ChannelBase::destroy()
{
    this->~ChannelBase();
    Free(this);
}

#include "tyrsound_end.h"
