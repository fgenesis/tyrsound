#ifndef TYRSOUND_INTERNAL_H
#define TYRSOUND_INTERNAL_H

#include <stdlib.h>
#include <string.h>
#include <new>

#include "tyrsound.h"

#include "tyrsound_begin.h"

// Definitions
class DecoderBase;
class DecoderFactoryBase;
class ChannelBase;
class DeviceBase;
class DeviceFactoryBase;
class SoundObject;
typedef DecoderBase *(*StreamLoaderFunc)(tyrsound_Stream);

// in tyrsound.cpp
extern tyrsound_Alloc g_alloc;

int lockUpdate();
void unlockUpdate();

class UpdateGuard
{
public:
    UpdateGuard() : _ok(lockUpdate()) {}
    ~UpdateGuard() { unlockUpdate(); }
    operator int() const { return _ok; }
private:
    int _ok;
};

// Generic inline functions

inline void *Realloc(void *ptr, size_t size) { return g_alloc(ptr, size); }
inline void  Free(void *ptr)                 { g_alloc(ptr, 0); }
inline void *Alloc(size_t size)              { return g_alloc(NULL, size); }

template<typename T>       T& Min(      T& a,       T& b) { return a < b ? a : b; }
template<typename T> const T& Min(const T& a, const T& b) { return a < b ? a : b; }

template<typename T>       T& Max(      T& a,       T& b) { return a > b ? a : b; }
template<typename T> const T& Max(const T& a, const T& b) { return a > b ? a : b; }


// in tyrsound_device.cpp
DeviceBase *getDevice();
bool initDevice(const char *name, const tyrsound_Format *fmt);
void shutdownDevice();

// in tyrsound_sound.cpp
tyrsound_Handle registerSoundObject(SoundObject *);
void shutdownSounds();
void updateSounds();

// Helper for static initialization
template <typename T, int SZ> class RegistrationHolder
{
public:
    RegistrationHolder() : _idx(0) {}
    static void Register(const T& item) { instance().add(item); }
    static const T& Get(size_t i) { return instance()._data[i]; }
    static size_t Size() { return instance()._idx; }

private:
    static RegistrationHolder<T, SZ>& instance()
    {
        static RegistrationHolder<T, SZ> holder;
        return holder;
    }
    void add(const T& item) { _data[_idx++] = item; }
    T _data[SZ];
    size_t _idx;
};


#include "tyrsound_end.h"
#endif

