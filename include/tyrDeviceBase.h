#ifndef TYRSOUND_DEVICE_BASE_H
#define TYRSOUND_DEVICE_BASE_H
#include "tyrsound_ex.h"
#include "tyrsound_begin.h"

class ChannelBase;

class DeviceBase
{
protected:

    DeviceBase() {}
    virtual ~DeviceBase() {}

public:
    void destroy();

    virtual tyrsound_Error setVolume(float vol) = 0;
    virtual tyrsound_Error setSpeed(float speed) = 0;
    virtual tyrsound_Error setPosition(float x, float y, float z) = 0;
    virtual void update() = 0;
    virtual ChannelBase *getFreeChannel() = 0;
};


class DeviceFactoryBase
{
public:
    virtual DeviceBase *create(tyrsound_Format& fmt) = 0;
};

template<class T> class DeviceFactory : public DeviceFactoryBase
{
    virtual DeviceBase *create(tyrsound_Format& fmt)
    {
        T *device = T::template create(fmt);
        return static_cast<DeviceBase*>(device);
    }
};

struct DeviceInfo
{
    DeviceInfo() {}
    DeviceInfo(DeviceFactoryBase *f, const char *n)
        : factory(f), name(n) {}
    DeviceFactoryBase *factory;
    const char *name;
};

template<typename T> struct DeviceRegistrar
{
    DeviceRegistrar(const char *name)
    {
        static DeviceFactory<T> instance;
        tyrsound_ex_registerDevice(DeviceInfo(&instance, name));
    }
};

#define TYRSOUND_REGISTER_DEVICE(name, type) \
    TYRSOUND_STATIC_REGISTER(DeviceRegistrar, type, (name))



#include "tyrsound_end.h"
#endif
