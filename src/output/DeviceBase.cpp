#include "tyrDeviceBase.h"

#include "tyrsound_begin.h"

void DeviceBase::destroy()
{
    this->~DeviceBase();
    Free(this);
}

#include "tyrsound_end.h"
