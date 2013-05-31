#ifndef TYRSOUND_DEVICE_BASE_H
#define TYRSOUND_DEVICE_BASE_H
#include "tyrsound.h"
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
    virtual void update() = 0;
    virtual ChannelBase *getFreeChannel() = 0;
};


class DeviceFactoryBase
{
public:
    virtual DeviceBase *create(tyrsound_Format& fmt) = 0;
};

template<typename T> class DeviceFactory : public DeviceFactoryBase
{
    virtual DeviceBase *create(tyrsound_Format& fmt)
    {
        T *device = typename T::create(fmt);
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


#define TYRSOUND_DEVICE_HOLDER RegistrationHolder<DeviceInfo, 16>

template<typename T> struct DeviceRegistrar
{
    DeviceRegistrar(const char *name)
    {
        static DeviceFactory<T> instance;
        typename TYRSOUND_DEVICE_HOLDER::Register(DeviceInfo(&instance, name));
    }
};

#define TYRSOUND_REGISTER_DEVICE(name, type) \
    TYRSOUND_STATIC_REGISTER(DeviceRegistrar, type, (name))



#include "tyrsound_end.h"
#endif
