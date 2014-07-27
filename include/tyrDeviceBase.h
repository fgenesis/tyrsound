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
    // The protocol is as follows:
    // First, reserve a channel if you plan to use one.
    // Then, either set it up & acquire it, or, if you changed your mind, retain it again.
    // When the channel is no longer needed, retain it.
    virtual ChannelBase *reserveChannel() = 0; // return a free channel, or NULL if no free channel could be found
    virtual void acquireChannel(ChannelBase *) = 0; // mark the channel as in use
    virtual void retainChannel(ChannelBase *) = 0; // once a channel is done, this is used so that it can be reserveChannel()'d again.
};


class DeviceFactoryBase
{
public:
    virtual DeviceBase *create(tyrsound_Format& fmt, tyrsound_DeviceConfig& cfg) = 0;
};

template<class T> class DeviceFactory : public DeviceFactoryBase
{
    typedef T K;
    virtual DeviceBase *create(tyrsound_Format& fmt, tyrsound_DeviceConfig& cfg)
    {
        T *device = K::create(fmt, cfg);
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
